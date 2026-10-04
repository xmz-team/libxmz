// test-aux.cc
#include <libxmz/fs.hpp>
#include <libxmz/aux.hpp>
#include <libxmz/io.hpp>

int main() {
    if (xmz::fs::touch("test.txt")) {
        xmz::println("the file was created successfully");
        if (xmz::aux::is_file("test.txt")) xmz::println("file test.txt exist");
        else xmz::perrln("file test.txt not exist");
        return 0;
    } else {
        xmz::perrln("failed to create the file");
        return 1;
    }
}
