/*
 * main.mm
 */

#include <string>

#include <libxmz/apple-ios-objc.hpp>
#include <libxmz/log.hpp>
#include <libxmz/fs.hpp>
#include <libxmz/aux.hpp>
#include <libxmz/crash.hpp>

#import <WebKit/WebKit.h>

namespace oc = xmz::ios::objc;

@interface WKTestDelegate : NSObject <WKNavigationDelegate>
@property (nonatomic, strong) UIViewController *hostVC;
@end

@implementation WKTestDelegate
- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
	xmz::log::info("WKWebView didFinishNavigation");
	[webView evaluateJavaScript:@"document.getElementById('msg').textContent" completionHandler:^(id result, NSError *err) {
		if (err)	xmz::log::error("evaluateJavaScript error:", err.localizedDescription.UTF8String);
		else		xmz::log::info("JS msg =", [result description].UTF8String);
	}];
}

- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation withError:(NSError *)error
{
	xmz::log::error("WKWebView didFailNavigation:", error.localizedDescription.UTF8String);
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation withError:(NSError *)error
{
	xmz::log::error("WKWebView didFailProvisional:", error.localizedDescription.UTF8String);
}
@end

/* AppDelegate */
@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic, strong) UIWindow *window;
@property (nonatomic, strong) WKTestDelegate *navDelegate;
@end

@implementation AppDelegate
- (BOOL)application:(UIApplication *)app didFinishLaunchingWithOptions:(NSDictionary *)opts
{
	xmz::log::info("didFinishLaunching");

	self.window = oc::make::window();
	UIViewController *vc = [[UIViewController alloc] init];
	vc.view.backgroundColor = oc::color::white();

	NSString *path = [[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@"www"];
	if (!path) {
		xmz::log::error("index.html not found in bundle");
		self.window.rootViewController = vc;
		[self.window makeKeyAndVisible];
		return YES;
	}
	NSURL *fileURL = [NSURL fileURLWithPath:path];
	NSURL *dirURL  = [fileURL URLByDeletingLastPathComponent];
	xmz::log::info("fileURL =", fileURL.absoluteString.UTF8String);
	xmz::log::info("dirURL	=", dirURL.absoluteString.UTF8String);

	WKWebViewConfiguration *cfg = [[WKWebViewConfiguration alloc] init];
	WKWebView *web = [[WKWebView alloc] initWithFrame:vc.view.bounds configuration:cfg];
	web.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

	self.navDelegate = [[WKTestDelegate alloc] init];
	self.navDelegate.hostVC = vc;
	web.navigationDelegate = self.navDelegate;

	[vc.view addSubview:web];

	[web loadFileURL:fileURL allowingReadAccessToURL:dirURL];

	self.window.rootViewController = vc;
	[self.window makeKeyAndVisible];
	return YES;
}
@end

int main(int argc, char *argv[])
{
	xmz::crash::init_crash_handler("/tmp/libxmz-test-wkwebview.crash.log");
	@autoreleasepool return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
}

