// test-a.mm
// build: c++ -framework UIKit -framework Foundation test-a.mm -o test-a

#include <libxmz/apple-ios-objc.hpp>
#include <libxmz/io.hpp>
#include <libxmz/log.hpp>
#include <libxmz/fs.hpp>
#include <libxmz/aux.hpp>
#include <libxmz/crash.hpp>

namespace ios { namespace oc = xmz::ios::objc; }

std::string jb = "/var/jb";
std::string mydata = jb + "/opt/libxmz/testapp-a/data";

@interface apprun : NSObject
- (BOOL)test;
@end

@implementation apprun
- (BOOL)test {
    if (xmz::aux::is_dir(mydata) == 1) {
        xmz::fs::mkdir(mydata);
    }
    // test write text
    if (xmz::fs::writefile(
        {
            "name: testapp-a",
            "bundle: io.github.xmz-team.libxmz.testapp-a",
            "version: 0.0.1"
        }, mydata + "Info.ini"
        )) {
        xmz::log::debug("write successfully!");
        return YES;
    } else {
        xmz::log::error("write failed!");
        return NO;
    }
}
@end

int main(int argc, char *argv[]) {
    xmz::crash::init_crash_handler("/var/log/io.github.xmz-team.libxmz.testapp-a.log");
    apprun *app = [[apprun alloc] init];
    [app test];
    [app release];
    return ios::oc::ui::run(argc, argv, [apprun class]);
}
