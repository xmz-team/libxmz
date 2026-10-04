#include <libxmz/io.hpp>

int main()
{
    xmz::println("hello, this println");
    xmz::fprintln(1, "hello, this fprintln out");
    xmz::fprintln(2, "hello, this fprintln err");
    return 0;
}
