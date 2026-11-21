// main.mm
#include <libxmz/apple-ios-objc.hpp>
#include <libxmz/log.hpp>
#include <libxmz/fs.hpp>
#include <libxmz/aux.hpp>
#include <libxmz/crash.hpp>
#import <WebKit/WebKit.h>
#include "fm.hpp"

namespace oc = xmz::ios::objc;

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic, strong) UIWindow *window;
@property (nonatomic, strong) FMHandler *fm;
@end

@implementation AppDelegate
- (BOOL)application:(UIApplication *)app
	didFinishLaunchingWithOptions:(NSDictionary *)opts
{
	xmz::crash::init("/var/mobile/Library/Logs/fm.crash.log");
	xmz::log::info("euid =", (int)geteuid(), "uid =", (int)getuid());

	self.window = oc::make::window();
	UIViewController *vc = [[UIViewController alloc] init];
	vc.view.backgroundColor = oc::color::white();

	WKWebViewConfiguration *cfg = [[WKWebViewConfiguration alloc] init];
	self.fm = [FMHandler new];
	[cfg.userContentController addScriptMessageHandler:self.fm name:@"fm"];

	WKWebView *web = [[WKWebView alloc]
					  initWithFrame:vc.view.bounds
					  configuration:cfg];
	web.autoresizingMask = UIViewAutoresizingFlexibleWidth |
						   UIViewAutoresizingFlexibleHeight;
	self.fm.web = web;
	[vc.view addSubview:web];

	NSString *path = [[NSBundle mainBundle]
					  pathForResource:@"index"
					  ofType:@"html"
					  inDirectory:@"www"];
	NSURL *fileURL = [NSURL fileURLWithPath:path];
	[web loadFileURL:fileURL
	 allowingReadAccessToURL:[fileURL URLByDeletingLastPathComponent]];

	self.window.rootViewController = vc;
	[self.window makeKeyAndVisible];
	return YES;
}
@end

int main(int argc, char *argv[])
{
	@autoreleasepool {
		return UIApplicationMain(argc, argv, nil,
								 NSStringFromClass([AppDelegate class]));
	}
}

