/*
 * aux.hpp
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

#ifndef XMZ_TEAM_AUX_HPP
#define XMZ_TEAM_AUX_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <memory>
#include <iostream>
#include <cstring>

#include <limits.h>

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

namespace xmz {
    // aux::exist and is_dir and is_file
    // return 0, it exists
    // return 1, it doesn't exist
    namespace aux {
        inline long long get_file_size(const std::string& path) { try { return std::filesystem::file_size(path); } catch (const std::filesystem::filesystem_error& e) { return -1; } }

        inline int exist(const char *path) {
            struct stat st;
            return (stat(path, &st) != 0); /* 0 if exists, 1 if not exists */
        }

        inline int is_dir(const char *path) {
            struct stat st;
            if (stat(path, &st) != 0) { return 1; /* doesn't exist or error */ }
            return (S_ISDIR(st.st_mode) ? 0 : 1);  /* 0 if is directory, 1 if not */
        }

        inline int is_file(const char *path) {
            struct stat st;
            if (stat(path, &st) != 0) { return 1; /* doesn't exist or error */ }
            return (S_ISREG(st.st_mode) ? 0 : 1);  // 0 if is file, 1 if not
        }

        template<typename T>
        inline int exist(const T& path) { return exist(path.c_str()); }
        template<typename T>
        inline int is_dir(const T& path) { return is_dir(path.c_str()); }
        template<typename T>
        inline int is_file(const T& path) { return is_file(path.c_str()); }
        template<typename T>
        inline int path_exist(const T& path) { return exist(path); }

        //constexpr auto& path_exist = exist;

        inline std::string resolve_path(const std::string& path) {
            char resolved_path[PATH_MAX];
            if (realpath(path.c_str(), resolved_path) != nullptr) {
                return std::string(resolved_path);
            }
            return path;
        }

        inline std::string resolve_path(const char* path) { return resolve_path(std::string(path)); }

        template<typename T>
        inline std::string parselink(const T& path) { return resolve_path(path); }
    } /* namespace aux */
    namespace auxiliary = aux;
} /* namespace xmz */
#endif /* XMZ_TEAM_AUX_HPP */
