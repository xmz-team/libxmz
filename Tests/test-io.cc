#include <libxmz/io.hpp>
#include <libxmz/log.hpp>
#include <string>
int main(int argc, char* argv[])
{
    // std::string XiaoMing = "XiaoMing";
    xmz::println("hello I is ", argv[0]);
    xmz::log::info("The message output is successful!");
    xmz::log::warn("This is a warning message");
    xmz::log::error("This is an error message");
    xmz::log::debug("This is a modulation message");
    return 0;
}
