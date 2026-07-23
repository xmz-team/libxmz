#include <libxmz/io.hpp>
#include <libxmz/fs.hpp>
#include <libxmz/str.hpp>
#include <string>

int main()
{
    std::string filename = "str-test.txt";
    std::string textcontent = "a = 1";
    xmz::println("this is str.hpp test");
    xmz::fs::touch(filename);
    xmz::fs::writefile(textcontent, filename);
    auto parts = xmz::str::split(xmz::fs::readfile_str(filename), "=");
    xmz::println("val a =", parts[1]);
    xmz::fs::rmfile(filename);
}
