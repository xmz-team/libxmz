// fm.hpp
#pragma once

#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>
#include <libxmz/fs.hpp>
#include <libxmz/aux.hpp>
#include <libxmz/log.hpp>
#include <libxmz/runsh.hpp>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

inline NSString *fm_abspath(NSString *p)
{
	if (!p || p.length == 0) p = @"/";
	if (![p isAbsolutePath]) p = [@"/" stringByAppendingPathComponent:p];
	return [[NSURL fileURLWithPath:p] URLByStandardizingPath].path;
}

inline bool fm_is_protected(NSString *canon)
{ return [canon isEqualToString:@"/"]; }

@interface FMHandler : NSObject <WKScriptMessageHandler>
@property (nonatomic, weak) WKWebView *web;
- (void)reply:(NSNumber *)rid ok:(BOOL)ok err:(NSString *)err;
- (void)reply:(NSNumber *)rid ok:(BOOL)ok data:(id)data;
- (void)reply:(NSNumber *)rid ok:(BOOL)ok data:(id)data err:(NSString *)err;
@end

@implementation FMHandler
- (void)userContentController:(WKUserContentController *)ucc
	  didReceiveScriptMessage:(WKScriptMessage *)msg
{
	if (![msg.name isEqualToString:@"fm"]) return;
	NSDictionary *req = msg.body;
	NSNumber *rid = req[@"id"];
	NSString *op  = req[@"op"];
	NSString *raw = req[@"path"] ?: @"/";

	@try {
		NSString *full = fm_abspath(raw);
		std::string p  = full.UTF8String;

		if ([op isEqualToString:@"list"]) {
			if (!xmz::aux::is_dir(p)) { [self reply:rid ok:NO err:@"not a dir"]; return; }
			NSMutableArray *arr = [NSMutableArray array];
			std::error_code ec;
			for (auto it = fs::directory_iterator(p,
						fs::directory_options::skip_permission_denied, ec);
					!ec && it != fs::directory_iterator();
					it.increment(ec)) {
				auto &e = *it;
				bool isd = e.is_directory(ec);
				long long sz = 0;
				if (!isd) { auto s = e.file_size(ec); if (!ec) sz = (long long)s; }
				[arr addObject:@{
					@"name": @(e.path().filename().c_str()),
					@"dir":  @(isd),
					@"size": @(sz),
					@"link": @(e.is_symlink(ec))
				}];
			}
			[self reply:rid ok:YES data:@{@"abs": full, @"entries": arr}];
		} else if ([op isEqualToString:@"stat"]) {
			std::error_code ec;
			if (!fs::exists(p, ec)) { [self reply:rid ok:NO err:@"not found"]; return; }
			bool isd = fs::is_directory(p, ec);
			auto sz  = isd ? 0LL : (long long)fs::file_size(p, ec);
			[self reply:rid ok:YES data:@{
				@"abs": full, @"dir": @(isd), @"size": @(sz),
				@"symlink": @(fs::is_symlink(p, ec))
			}];
		} else if ([op isEqualToString:@"read"]) {
			long long sz = xmz::aux::get_file_size(p);
			if (sz < 0) { [self reply:rid ok:NO err:@"not a file"]; return; }
			if (sz > 4*1024*1024) {
				[self reply:rid ok:NO err:@"too large (>4MB)"];
				return;
			}
			std::string s = xmz::fs::readfile_str(p);
			[self reply:rid ok:YES data:s.empty()? @"" : @(s.c_str())];
		} else if ([op isEqualToString:@"write"]) {
			bool ok = xmz::fs::writefile([req[@"data"] UTF8String], p);
			[self reply:rid ok:ok err:ok?nil:@"write failed"];
		} else if ([op isEqualToString:@"mkdir"]) {
			bool ok = xmz::fs::mkdir(p);
			[self reply:rid ok:ok err:ok?nil:@"mkdir failed"];
		} else if ([op isEqualToString:@"rm"]) {
			if (fm_is_protected(full)) {
				[self reply:rid ok:NO err:@"refuse to rm /"];
				return;
			}
			bool ok = xmz::aux::is_dir(p) ? xmz::fs::recrmdir(p)
										  : xmz::fs::rmfile(p);
			[self reply:rid ok:ok err:ok?nil:@"rm failed"];
		} else if ([op isEqualToString:@"rename"]) {
			NSString *to = fm_abspath(req[@"to"]);
			if (fm_is_protected(full)) { [self reply:rid ok:NO err:@"refuse"]; return; }
			bool ok = xmz::fs::rename(p, to.UTF8String);
			[self reply:rid ok:ok err:ok?nil:@"rename failed"];
		} else if ([op isEqualToString:@"cp"]) {
			NSString *to = fm_abspath(req[@"to"]);
			bool isd = xmz::aux::is_dir(p);
			bool ok = xmz::fs::cp(isd ? xmz::fs::cptype::recdir
									  : xmz::fs::cptype::file,
								  p, to.UTF8String);
			[self reply:rid ok:ok err:ok?nil:@"cp failed"];
		} else if ([op isEqualToString:@"symlink"]) {
			NSString *to = fm_abspath(req[@"to"]);
			bool ok = xmz::fs::mklnfile(p, to.UTF8String);
			[self reply:rid ok:ok err:ok?nil:@"symlink failed"];
		} else if ([op isEqualToString:@"sh"]) {
			auto r = xmz::cmd::runsh_capture([req[@"cmd"] UTF8String]);
			[self reply:rid ok:YES data:@{
				@"code": @(r.exit_code),
				@"out":  @(r.stdout_output.c_str()),
				@"err":  @(r.stderr_output.c_str())
			}];
		} else {
			[self reply:rid ok:NO err:@"unknown op"];
		}
	} @catch (NSException *ex) {
		xmz::log::error("fm exception:", ex.reason.UTF8String);
		[self reply:rid ok:NO err:ex.reason];
	}
}

- (void)reply:(NSNumber *)rid ok:(BOOL)ok data:(id)data err:(NSString *)err
{
	NSMutableDictionary *r = [NSMutableDictionary dictionary];
	r[@"id"] = rid ?: @0;
	r[@"ok"] = @(ok);
	if (data) r[@"data"] = data;
	if (err)  r[@"err"]  = err;

	NSData *j = [NSJSONSerialization dataWithJSONObject:r options:0 error:nil];
	NSString *js = [NSString stringWithFormat:@"window.__fmCb(%@)",
					[[NSString alloc] initWithData:j encoding:NSUTF8StringEncoding]];
	dispatch_async(dispatch_get_main_queue(), ^{
		[self.web evaluateJavaScript:js completionHandler:nil];
	});
}

- (void)reply:(NSNumber *)rid ok:(BOOL)ok err:(NSString *)err
{ [self reply:rid ok:ok data:nil err:err]; }
- (void)reply:(NSNumber *)rid ok:(BOOL)ok data:(id)data
{ [self reply:rid ok:ok data:data err:nil]; }
@end

