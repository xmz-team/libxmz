/*
 * runsh.hpp
 * Created by XMZ Studio and xhzer <xhzer@outlook.com> on 2026-11-18
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

#ifndef LIBXMZ_RUNSH_HPP
#define LIBXMZ_RUNSH_HPP
#include "_runsh.hpp"
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <cstring>

namespace xmz::cmd {
	/* execute shell but do not capture output */
	inline int run_shell(const std::string& command,
						 _runsh::_::shell_type type = _runsh::_::shell_type::sh)
	{
		if (command.empty()) return 1;
		const char* shell_path = _runsh::_::find_shell_path(type);
		const char* shell_name = _runsh::_::get_shell_name(type);
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

	inline int runsh(const std::string& command)
	{ return run_shell(command, _runsh::_::shell_type::sh); }
	inline int runbash(const std::string& command)
	{ return run_shell(command, _runsh::_::shell_type::bash); }

	struct command_result {
		int exit_code;
		std::string stdout_output;
		std::string stderr_output;
	};
	// execute shell and capture output
	inline command_result
	run_shell_capture(const std::string& command,
					  _runsh::_::shell_type type = _runsh::_::shell_type::sh)
	{
		command_result result{0, "", ""};
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
			close(stdout_pipe[0]); 
			close(stderr_pipe[0]);
			dup2(stdout_pipe[1], STDOUT_FILENO);
			dup2(stderr_pipe[1], STDERR_FILENO);
			close(stdout_pipe[1]); 
			close(stderr_pipe[1]);
			const char* shell_path = _runsh::_::find_shell_path(type);
			const char* shell_name = _runsh::_::get_shell_name(type);
			execl(shell_path, shell_name, "-c", command.c_str(), (char*)nullptr);
			_exit(127);
		}

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
			struct timeval timeout = {1, 0};
			int ret = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);
			if (ret == -1) break;
			if (ret == 0) continue; // timeout, continue to cycle
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
		auto trim_newline = [](std::string& s) {
			while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
				s.pop_back();
		};
		trim_newline(result.stdout_output);
		trim_newline(result.stderr_output);
		return result;
	}

	inline command_result runsh_capture(const std::string& command)
	{ return run_shell_capture(command, _runsh::_::shell_type::sh); }
	inline command_result runbash_capture(const std::string& command)
	{ return run_shell_capture(command, _runsh::_::shell_type::bash); }
}
#endif /* LIBXMZ_RUNSH_HPP */

