#include <libxmz/exec.hpp>
#include <libxmz/log.hpp>
#include <cerrno>
#include <cstring>

int main() {
    int ret = xmz::exec::execvp(nullptr, "ls", "-l", "-a", nullptr);
    if (ret != 0) {
        xmz::log::error("execv failed: %s (errno: %d)", strerror(ret), ret);
        return 1;
    }
    return 0;
}
