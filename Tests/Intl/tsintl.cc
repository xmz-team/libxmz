#include <libxmz/io.hpp>
#include <libxmz/str.hpp>
#include <libxmz/fs.hpp>

int main() {
    auto config_parts = xmz::str::split(xmz::fs::readfile_str("config.ini"), "=");
    auto intl_parts = xmz::str::split(xmz::fs::readfile_str("intl.ini"), "=");

}