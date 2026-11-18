/*
 * fs.hpp
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

#ifndef LIBXMZ_FS_HPP
#define LIBXMZ_FS_HPP
#include <filesystem>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "io.hpp"
#include "_fs.hpp"

namespace xmz {
	namespace fs {
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

		inline const auto& emptyfile = touch;

		inline bool writefile(const std::string& text, const std::string& file) {
			std::ofstream outfile(file);
			if (!outfile.is_open()) return false;
			outfile << text << '\n';
			return true;
		}
		template<typename Container>
		inline bool writefile(const Container& lines, const std::string& file) {
			std::ofstream outfile(file);
			if (!outfile.is_open()) return false;
			for (const auto& line : lines) outfile << line << '\n';
			return true;
		}
		inline bool writefile(std::initializer_list<std::string> lines, const std::string& file) {
			std::ofstream outfile(file);
			if (!outfile.is_open()) return false;
			for (const auto& line : lines) outfile << line << '\n';
			return true;
		}
		inline bool
		writefile(const char *lines, const std::string& file) { return writefile(std::string(lines), file); }

		inline bool __attribute__((deprecated))
		readfile(const std::string& file)
		{
			std::ifstream file_path(file);
			std::string line;
			if (!file_path.is_open()) return false;
			while (std::getline(file_path, line)) xmz::println(line);
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

		inline bool rmfile(const std::string& filename)
		{ return xmz::fs::_::remove(filename); }
		inline bool rmdir(const std::string& dirname)
		{ return xmz::fs::_::remove(dirname); }
		inline bool recrmdir(const std::string& dirname)
		{ bool rec = true; return xmz::fs::_::remove(dirname, rec); }

		using cptype = _::cp::type;
		inline bool cp(_::cp::type type,
					   const std::string& orig_name,
					   const std::string& current_name)
		{ return _::cp::cp(type, orig_name, current_name); }

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

		inline bool
		findstr(const std::string& file, const std::string& keyword)
		{ return _::findstr(file, keyword, false); }
		inline bool
		fmfindstr(const std::string& file, const std::string& keyword)
		{ return _::findstr(file, keyword, true); }

		inline bool
		rmfilestr(const std::string& filename, const std::string& target)
		{ return _::remove_str_from_file(filename, target); }
		inline bool
		rmtextline(const std::string& filename, const std::string& keyword)
		{ return _::remove_text_line_from_file(filename, keyword); }

		inline bool
		mklnfile(const std::string& target, const std::string& link_name)
		{ return _::mklink(target, link_name, true); }
		inline bool
		mklndir(const std::string& target, const std::string& link_name)
		{ return _::mklink(target, link_name, false); }

		inline _::findtext::findresult
		fmfindresult(const std::string& file, const std::string& keyword)
		{ return _::findtext::findresult(file, keyword, false); }
		inline _::findtext::findresult
		findresult(const std::string& file, const std::string& keyword)
		{ return _::findtext::findresult(file, keyword, true); }

		inline bool
		rename(const std::string& orig_name, const std::string& new_name)
		{
			try {
				std::filesystem::rename(orig_name, new_name);
				return true;
			} catch (const std::filesystem::filesystem_error& e) {
				return false;
			}
		}

		inline bool
		append(const std::string& text, const std::string& file)
		{
			std::ofstream outfile(file, std::ios::app);
			if (!outfile.is_open()) return false;
			outfile << text << '\n';
			return true;
		}
	} // namespace fs
} // namespace xmz
#endif // LIBXMZ_FS_HPP

