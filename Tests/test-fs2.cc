// test-fs2.cc
#include <libxmz/fs.hpp>
#include <libxmz/io.hpp>
#include <libxmz/log.hpp>

int main() {
    xmz::fs::writefile(
        {
            "1234",
            "5678"
        },
        "test.txt"
    );
    xmz::log::info("orig");
    xmz::fs::readfile("test.txt");
    if (xmz::fs::rmfilestr("test.txt", "1234")) { xmz::log::info("remove 1234 success"); } else { xmz::log::error("remove 1234 failed"); }
    xmz::log::info("now");
    xmz::fs::readfile("test.txt");
    if (xmz::fs::rmfile("test.txt") { xmz::log::info("remove file: test.txt success"); } else { xmz::log::error("remove file: test.txt failed"); }
    return 0;
}
