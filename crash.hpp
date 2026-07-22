/*
 * crash.hpp
 * Created by XMZ <xmz-team@outlook.com> on 20/7/26
 * Copyright (c) 2026 XMZ <xmz-team@outlook.com> All rights reserved.
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
// 全局可配的日志路径, 默认为空, 输出到 stderr
static const char *g_crash_log_path = nullptr;
// 防止信号处理函数重入或多线程同时执行
static std::atomic_flag g_crash_lock = ATOMIC_FLAG_INIT;
// 初始化函数, 在程序启动时调用
void init_crash_handler(const char *log_path = nullptr) {
    g_crash_log_path = log_path;
    // 注册常见 crash 信号
    signal(SIGSEGV, sig_handler);
    signal(SIGABRT, sig_handler);
    signal(SIGFPE,  sig_handler);
    signal(SIGILL,  sig_handler);
    signal(SIGBUS,  sig_handler);
}
// 通用的信号处理函数
void sig_handler(int sig) {
    // 如果已有一个 handler 在执行, 直接调用默认处理避免死锁
    if (g_crash_lock.test_and_set(std::memory_order_acquire)) {
        signal(sig, SIG_DFL);
        raise(sig);
        return;
    }
    // 立即向 stderr 输出
    const char *msg = "CRASH\n";
    write(STDERR_FILENO, msg, strlen(msg));
    // 获取调用栈
    void *callstack[128];
    int frames = backtrace(callstack, 128);
    char **strs = backtrace_symbols(callstack, frames);
    // 根据配置决定输出目标
    int fd = -1;
    if (g_crash_log_path && g_crash_log_path[0] != '\0') {
        fd = open(g_crash_log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    }
    // 若打开失败或未配置文件, 直接退回到 stderr
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
} /* namespace xmz::crash */
#endif /* XMZ_TEAM_CRASH_HPP */
