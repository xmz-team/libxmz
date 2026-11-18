/*
 * _runsh.hpp
 * Created by XMZ Studio and xhz <xhzer@outlook.com> on 2026-11-16
 * Copyright (c) 2026 XMZ Studio and xhz <xhzer@outlook.com>
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

#ifndef _LIBXMZ_RUNSH_HPP
#define _LIBXMZ_RUNSH_HPP
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>

namespace xmz::cmd {
	namespace _runsh {
		enum class shell_type {
			sh,
			bash
		};
	
		inline const char* sh_path[] = {
			"/var/jb/usr/bin/sh",
			"/var/jb/bin/sh",
			"/var/jb/usr/local/bin/sh",
			"/bin/sh",
			"/usr/bin/sh",
			"/usr/local/bin/sh",
			nullptr
		};
	
		inline const char* bash_path[] = {
			"/var/jb/usr/bin/bash",
			"/var/jb/bin/bash",
			"/var/jb/usr/local/bin/bash",
			"/bin/bash",
			"/usr/bin/bash",
			"/usr/local/bin/bash",
			nullptr
		};
	
		inline const char* find_shell_path(shell_type type)
		{
			const char* const* paths = (type == shell_type::bash) ? bash_path : sh_path;
			for (int i = 0; paths[i] != nullptr; i++)
				if (access(paths[i], F_OK) == 0) return paths[i];
			return "/bin/sh";  /* fallback */
		}
	
		inline const char* get_shell_name(shell_type type)
		{ return (type == shell_type::bash) ? "bash" : "sh"; }
	} /* namespace cmd::runsh::_ */
} /* namespace xmz::cmd */
#endif /* _LIBXMZ_RUNSH_HPP */
