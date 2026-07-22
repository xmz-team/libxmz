#include "io.hpp"

int main()
{
    xmz::println("hello, this println");
    xmz::fprintln("stdout", "hello, this fprintln out");
    xmz::fprintln("stderr", "hello, this fprintln err");
    return 0;
}
