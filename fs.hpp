/*
 * fs.hpp
 * Created by XMZ <xmz-team@outlook.com> on 10/7/04
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

#ifndef XMZ_TEAM_FS_HPP
#define XMZ_TEAM_FS_HPP

#include <filesystem>
#include <string>
#include <fstream>
#include <iostream>
#include <libxmz/io.hpp>
#include <sstream>

namespace xmz {
    namespace _fs {
        inline void remove(const std::string& name, bool rec = false) {
            if (rec) {
                std::filesystem::remove_all(name);
            } else {
                std::filesystem::remove(name);
            }
        }
    }
    namespace fs {
        /* I thought about using define directly, but because define is global, even in the namespace, I gave up using define to write */
        inline void mkdir(const std::string& path) { std::filesystem::create_directories(path); }
        inline void touch(const std::string& filename) { std::ofstream(filename).close(); }
        constexpr auto& emptyfile = touch;
        inline bool writefile(const std::string& text, const std::string& file) {
            std::ofstream outfile(file);
            if (!outfile.is_open()) { return false; }
            outfile << text << '\n';
            return true;
        }
        inline bool readfile(const std::string& file) {
            std::ifstream file_path(file);
            std::string line;
            if (!file_path.is_open()) { return false; }
            while (std::getline(file_path, line)) { xmz::print(line); }
            file_path.close();
            return true;
        }
        inline std::string readfile_str(const std::string& file) {
            std::ifstream file_path(file);
            if (!file_path.is_open()) return "";
    
            std::stringstream buffer;
            buffer << file_path.rdbuf();
            return buffer.str();
        }
        inline void rmfile(const std::string& filename) { return xmz::_fs::remove(filename); }
        inline void rmdir(const std::string& dirname) { return xmz::_fs::remove(dirname); }
        inline void recrmdir(const std::string& dirname) { bool rec = true; return xmz::_fs::remove(dirname, rec); }
    } // namespace fs
} // namespace xmz

#endif // XMZ_TEAM_FS_HPP
