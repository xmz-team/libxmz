#include <libxmz/fs.hpp>

int main() {
    xmz::fs::touch("test.txt");
    xmz::fs::cp(xmz::fs::cptype::file, "test.txt", "test.txt.1");
    xmz::fs::rmfile("test.txt");
    xmz::fs::rmfile("test.txt.1");
    return 0;
}
