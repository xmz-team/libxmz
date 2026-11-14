// test-io-auto-space.cc
#define LIBXMZ_IO_NO_AUTO_SPACE 0
#include <libxmz/io.hpp>
#include <string>
int main() {
    std::string a = "1";
    xmz::println("1", "+", a, "=", "2");
    return 0;
}
