/*
 * fs.hpp
 * Created by XMZ <xmz-team@outlook.com> on 10/7/04
 * Copyright (c) 2025-2026 XMZ <xmz-team@outlook.com> All rights reserved.
 */

#ifndef XMZ_TEAM_FS_HPP
#define XMZ_TEAM_FS_HPP

#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <string>

namespace xmz {
    namespace fs {
        /* In fact, I want to use macros to write these two functions, but considering that macros are global even if they are defined in the namespace, they give up. */
        inline void mkdir(const std::string& path) { std::filesystem::create_directories(path); }
        inline void touch(const std::string& filename) { std::ofstream(filename).close(); }
        constexpr auto& emptyfile = touch;
    } // namespace fs
} // namespace xmz

#endif // XMZ_TEAM_FS_HPP
