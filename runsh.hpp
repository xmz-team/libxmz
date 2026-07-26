/*
 * runsh.hpp
 * Created by XMZ <xmz-team@outlook.com> on 10/7/26
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

#ifndef XMZ_RUNSH_HPP
#define XMZ_RUNSH_HPP

#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>

namespace xmz::cmd {
    enum class ShellType {
        SH,
        BASH
    };

    static const char* SHELL_PATHS[] = {
        "/var/jb/usr/bin/sh",
        "/var/jb/bin/sh",
        "/var/jb/usr/local/bin/sh",
        "/bin/sh",
        "/usr/bin/sh",
        "/usr/local/bin/sh",
        nullptr
    };

    static const char* BASH_PATHS[] = {
        "/var/jb/usr/bin/bash",
        "/var/jb/bin/bash",
        "/var/jb/usr/local/bin/bash",
        "/bin/bash",
        "/usr/bin/bash",
        "/usr/local/bin/bash",
        nullptr
    };

    inline const char* find_shell_path(ShellType type) {
        const char* const* paths = (type == ShellType::BASH) ? BASH_PATHS : SHELL_PATHS;
        for (int i = 0; paths[i] != nullptr; i++) {
            if (access(paths[i], F_OK) == 0) {
                return paths[i];
            }
        }
        return "/bin/sh";  // fallback
    }

    inline const char* get_shell_name(ShellType type) {
        return (type == ShellType::BASH) ? "bash" : "sh";
    }
    // 通用执行函数（不捕获输出）
    inline int run_shell(const std::string& command, ShellType type = ShellType::SH) {
        if (command.empty()) return 1;
        const char* shell_path = find_shell_path(type);
        const char* shell_name = get_shell_name(type);
        pid_t pid = fork();
        if (pid == -1) return -1;
        if (pid == 0) {
            execl(shell_path, shell_name, "-c", command.c_str(), (char*)nullptr);
            _exit(127);
        }

        int status;
        if (waitpid(pid, &status, 0) == -1) return -1;
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
    // 保持向后兼容
    inline int runsh(const std::string& command) {
        return run_shell(command, ShellType::SH);
    }

    inline int runbash(const std::string& command) {
        return run_shell(command, ShellType::BASH);
    }
    // 捕获输出的结果结构
    struct CommandResult {
        int exit_code;
        std::string stdout_output;
        std::string stderr_output;
    };
    // 通用捕获输出函数
    inline CommandResult run_shell_capture(const std::string& command, 
                                          ShellType type = ShellType::SH) {
        CommandResult result{0, "", ""};
        int stdout_pipe[2], stderr_pipe[2];
        if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
            result.exit_code = -1;
            return result;
        }

        pid_t pid = fork();
        if (pid == -1) {
            close(stdout_pipe[0]); close(stdout_pipe[1]);
            close(stderr_pipe[0]); close(stderr_pipe[1]);
            result.exit_code = -1;
            return result;
        }

        if (pid == 0) {
            // 子进程
            close(stdout_pipe[0]); 
            close(stderr_pipe[0]);
            dup2(stdout_pipe[1], STDOUT_FILENO);
            dup2(stderr_pipe[1], STDERR_FILENO);
            close(stdout_pipe[1]); 
            close(stderr_pipe[1]);
            
            const char* shell_path = find_shell_path(type);
            const char* shell_name = get_shell_name(type);
            execl(shell_path, shell_name, "-c", command.c_str(), (char*)nullptr);
            _exit(127);
        }
        // 父进程
        close(stdout_pipe[1]); 
        close(stderr_pipe[1]);

        char buffer[4096];
        ssize_t count;
        bool stdout_closed = false;
        bool stderr_closed = false;

        while (!stdout_closed || !stderr_closed) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            
            int max_fd = -1;
            if (!stdout_closed) {
                FD_SET(stdout_pipe[0], &read_fds);
                max_fd = stdout_pipe[0];
            }
            if (!stderr_closed) {
                FD_SET(stderr_pipe[0], &read_fds);
                max_fd = std::max(max_fd, stderr_pipe[0]);
            }
            
            if (max_fd == -1) break;
            
            struct timeval timeout = {1, 0}; // 1秒超时
            int ret = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);
            
            if (ret == -1) break;
            if (ret == 0) continue; // 超时,继续循环
            
            if (!stdout_closed && FD_ISSET(stdout_pipe[0], &read_fds)) {
                count = read(stdout_pipe[0], buffer, sizeof(buffer) - 1);
                if (count > 0) {
                    buffer[count] = '\0';
                    result.stdout_output += buffer;
                } else {
                    stdout_closed = true;
                }
            }
            
            if (!stderr_closed && FD_ISSET(stderr_pipe[0], &read_fds)) {
                count = read(stderr_pipe[0], buffer, sizeof(buffer) - 1);
                if (count > 0) {
                    buffer[count] = '\0';
                    result.stderr_output += buffer;
                } else {
                    stderr_closed = true;
                }
            }
        }

        close(stdout_pipe[0]); 
        close(stderr_pipe[0]);

        int status;
        waitpid(pid, &status, 0);
        result.exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        // 去除末尾换行
        auto trim_newline = [](std::string& s) {
            while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
                s.pop_back();
            }
        };
        trim_newline(result.stdout_output);
        trim_newline(result.stderr_output);

        return result;
    }
    // 保持向后兼容的捕获函数
    inline CommandResult runsh_capture(const std::string& command) {
        return run_shell_capture(command, ShellType::SH);
    }

    inline CommandResult runbash_capture(const std::string& command) {
        return run_shell_capture(command, ShellType::BASH);
    }
}

#endif // XMZ_RUNSH_HPP
