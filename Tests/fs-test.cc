#include <libxmz/io.hpp>
#include <libxmz/fs.hpp>
#include <libxmz/aux.hpp>
#include <string>

int main() {
    std::string filename = "libxmz-test-fs.txt";
    std::string writetext = "hello";
    std::string readtext = xmz::fs::readfile_str(filename);
    xmz::println("libxmz fs test");
    xmz::println("test writefile");
    xmz::fs::touch(filename);
    if (xmz::aux::is_file(filename.c_str()) == 0) {
        xmz::println("file: ", filename, " created successfully");
    } else {
        xmz::println("file: ", filename, " created failed");
    }

    xmz::fs::writefile(writetext, filename);

    if (readtext == writetext) {
        xmz::println("write successfully");
    } else {
        xmz::println("write failed");
    }

    // xmz::fs::readfile(filename);
    xmz::fs::rmfile(filename);
    if (xmz::aux::is_file(filename.c_str()) == 1) {
        xmz::println("remove successfully");
    } else {
        xmz::println("remove failed");
    }
    return 0;
}
