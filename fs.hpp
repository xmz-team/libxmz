/*
 * fs.hpp
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
        inline bool remove(const std::string& name, bool rec = false) {
            try {
                if (rec) {
                    std::filesystem::remove_all(name); 
                } else {
                    std::filesystem::remove(name);
                }
                return true;
            } catch (const std::filesystem::filesystem_error&) {
                return false;
            }
        }
        namespace cp {
            enum class copy_type { 
                file,         // copy file
                dir,          // copy dir
                recdir, // rec copy dir
                link          // copy link
            };

            inline bool cp(copy_type type,
                   const std::string& orig_name, 
                   const std::string& current_name) {
                try {
                    switch(type) {
                        case copy_type::file:
                            std::filesystem::copy(orig_name, current_name, 
                            std::filesystem::copy_options::overwrite_existing);
                            break;
                        case copy_type::recdir:
                            std::filesystem::copy(orig_name, current_name, 
                            std::filesystem::copy_options::recursive | 
                            std::filesystem::copy_options::overwrite_existing);
                            break;
                        case copy_type::dir:
                            std::filesystem::copy(orig_name, current_name);
                            break;
                        case copy_type::link:
                            std::filesystem::copy(orig_name, current_name, 
                            std::filesystem::copy_options::copy_symlinks);
                            break;
                    }
                    return true;
                } catch (const std::filesystem::filesystem_error&) {
                    return false;
                }
            }
        } /* namespace cp */
    } /* namespace _fs */
    namespace fs {
        /* I thought about using define directly, but because define is global, even in the namespace, I gave up using define to write */
        inline bool mkdir(const std::string& path) {
            try {
                std::filesystem::create_directories(path);
                return true;
            } catch (const std::filesystem::filesystem_error&) {
                return false;
            }
        }
        inline bool touch(const std::string& filename) {
            try {
                std::ofstream file(filename);
                if (!file) return false;
                file.close();
                return true;
            } catch (const std::exception&) {
                return false;
            }
        }
        const auto& emptyfile = touch;
        inline bool writefile(const std::string& text, const std::string& file) {
            std::ofstream outfile(file);
            if (!outfile.is_open()) { return false; }
            outfile << text << '\n';
            return true;
        }
        template<typename Container>
        inline bool writefile(const Container& lines, const std::string& file) {
            std::ofstream outfile(file);
            if (!outfile.is_open()) return false;
            for (const auto& line : lines) {
                outfile << line << '\n';
            }
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
        inline bool rmfile(const std::string& filename) { return xmz::_fs::remove(filename); }
        inline bool rmdir(const std::string& dirname) { return xmz::_fs::remove(dirname); }
        inline bool recrmdir(const std::string& dirname) { bool rec = true; return xmz::_fs::remove(dirname, rec); }

        using cptype = _fs::cp::copy_type;
        inline bool cp(_fs::cp::copy_type type,
               const std::string& orig_name, 
               const std::string& current_name) { return _fs::cp::cp(type, orig_name, current_name); }
        inline std::string pwd() {
            try {
                std::filesystem::path currentdir = std::filesystem::current_path();
                return currentdir;
            } catch (const std::filesystem::filesystem_error&) {
                return "";
            }
        }

        inline bool cd(const std::string& path) {
            try {
                std::filesystem::current_path(path);
                return true;
            } catch (const std::filesystem::filesystem_error&) {
                return false;
            }
        }
    } // namespace fs
} // namespace xmz

#endif // XMZ_TEAM_FS_HPP
