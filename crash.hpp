/*
 * crash.hpp
 * Created by XMZ <xmz-team@outlook.com> on 20/7/26
 * Copyright (c) 2026 XMZ <xmz-team@outlook.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
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

#ifndef XMZ_TEAM_CRASH_HPP
#define XMZ_TEAM_CRASH_HPP

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <atomic>

namespace xmz::crash {
// globally available log path, empty by default, output to stderr
static const char *g_crash_log_path = nullptr;
// prevent signal processing function re-entry or multi-thread simultaneous execution
static std::atomic_flag g_crash_lock = ATOMIC_FLAG_INIT;
// general signal processing function
void sig_handler(int sig) {
    // If there is already a handler executing, call the default processing directly to avoid deadlock
    if (g_crash_lock.test_and_set(std::memory_order_acquire)) {
        signal(sig, SIG_DFL);
        raise(sig);
        return;
    }
    // output to stderr immediately
    const char *msg = "CRASH\n";
    write(STDERR_FILENO, msg, strlen(msg));
    // get the call stack
    void *callstack[128];
    int frames = backtrace(callstack, 128);
    char **strs = backtrace_symbols(callstack, frames);
    // determine the output target according to the configuration
    int fd = -1;
    if (g_crash_log_path && g_crash_log_path[0] != '\0') {
        fd = open(g_crash_log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    }
    // If you fail to open or do not have a configuration file, return directly to stderr
    int out_fd = (fd >= 0) ? fd : STDERR_FILENO;
    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "CRASH at %ld\nSignal: %d (%s)\n",
                       time(NULL), sig, strsignal(sig));
    write(out_fd, buffer, len);
    for (int i = 0; i < frames; i++) {
        if (strs[i]) {
            write(out_fd, "  ", 2);
            write(out_fd, strs[i], strlen(strs[i]));
            write(out_fd, "\n", 1);
        }
    }
    if (fd >= 0) { close(fd); }
    free(strs);
    signal(sig, SIG_DFL);
    raise(sig);
}
// Initialization function, called when the program starts
void init_crash_handler(const char *log_path = nullptr) {
    g_crash_log_path = log_path;
    // register common crash signals
    signal(SIGSEGV, sig_handler);
    signal(SIGABRT, sig_handler);
    signal(SIGFPE,  sig_handler);
    signal(SIGILL,  sig_handler);
    signal(SIGBUS,  sig_handler);
}
} /* namespace xmz::crash */
#endif /* XMZ_TEAM_CRASH_HPP */
