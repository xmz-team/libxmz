#include <libxmz/io.hpp>
#include <libxmz/log.hpp>
#include <string>

int main(int argc, char *argv[]) {
    std::string name = argv[0];
    xmz::log::info("hello I is ", name.c_str());
    xmz::log::warn("this is one warn msg");
    xmz::log::error("this is one error msg");
    xmz::log::debug("this is one debug msg");
    return 0;
}
