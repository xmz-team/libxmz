#include <libxmz/io.hpp>
#include <libxmz/log.hpp>
#include <string>

int main(int argc, char* argv[])
{
    xmz::println("hello I am", argv[0]);
    xmz::log::info("The message output is successful!");
    xmz::log::info("this is info msg");
    xmz::log::warn("this is warn msg");
    xmz::log::error("this is error msg");
    xmz::log::debug("this is debug msg");
    xmz::log::emerg("this is emerg msg");
    xmz::log::crit("this is crit msg");
    xmz::log::alert("this is alert msg");
    xmz::log::notice("this is notice msg");
    return 0;
}
