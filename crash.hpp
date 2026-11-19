/*
 * crash.hpp
 * Copyright (c) 2026 XMZ Studio
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 */

#ifndef LIBXMZ_CRASH_HPP
#define LIBXMZ_CRASH_HPP
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <atomic>
#include <string>

namespace xmz::crash {
namespace _ {
inline char g_crash_log_path[PATH_MAX] = {0};
inline std::atomic_flag g_crash_lock = ATOMIC_FLAG_INIT;

inline void set_log_path(const char *path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        g_crash_log_path[0] = '\0';
        return;
    }
    const size_t n   = std::strlen(path);
    const size_t cap = sizeof(g_crash_log_path) - 1;
    const size_t len = (n < cap) ? n : cap;
    std::memcpy(g_crash_log_path, path, len);
    g_crash_log_path[len] = '\0';
}

inline void sig_handler(int sig) noexcept;

inline void install_one(int sig) noexcept {
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND | SA_NODEFER;
    sigaction(sig, &sa, nullptr);
}
} /* namespace _ */

inline void _::sig_handler(int sig) noexcept {
    if (_::g_crash_lock.test_and_set(std::memory_order_acquire)) {
        signal(sig, SIG_DFL);
        raise(sig);
        return;
    }

    int fd = -1;
    if (_::g_crash_log_path[0] != '\0')
        fd = ::open(_::g_crash_log_path,
                    O_WRONLY | O_CREAT | O_APPEND, 0644);
    int out_fd = (fd >= 0) ? fd : STDERR_FILENO;

    const char *head = "CRASH\n";
    ::write(out_fd, head, std::strlen(head));

    char buf[32];
    int  len = 0;
    const char *pfx = "Signal: ";
    for (const char *p = pfx; *p; ++p) buf[len++] = *p;

    char digits[8];
    int  dn = 0, v = sig;
    if (v == 0) digits[dn++] = '0';
    while (v > 0) { digits[dn++] = char('0' + v % 10); v /= 10; }
    while (dn > 0) buf[len++] = digits[--dn];
    buf[len++] = '\n';
    ::write(out_fd, buf, len);

    void *callstack[128];
    int frames = backtrace(callstack, 128);
    backtrace_symbols_fd(callstack, frames, out_fd);

    if (fd >= 0) ::close(fd);

    signal(sig, SIG_DFL);
    raise(sig);
}

inline void init(const char *log_path = nullptr) {
    _::set_log_path(log_path);
    _::install_one(SIGSEGV);
    _::install_one(SIGABRT);
    _::install_one(SIGFPE);
    _::install_one(SIGILL);
    _::install_one(SIGBUS);
}

inline void init(const std::string &log_path)
{ init(log_path.c_str()); }

inline void __attribute__((deprecated))
init_crash_handler(const char *log_path = nullptr)
{ init(log_path); }

inline void __attribute__((deprecated))
init_crash_handler(const std::string &log_path)
{ init(log_path); }
} /* namespace xmz::crash */
#endif /* LIBXMZ_CRASH_HPP */
