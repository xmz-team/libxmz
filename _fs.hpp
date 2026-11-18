/*
 * _fs.hpp
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

#ifndef _LIBXMZ_FS_HPP
#define _LIBXMZ_FS_HPP
#include <filesystem>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace xmz {
	namespace fs::_ {
		namespace stdfs = std::filesystem;
		inline bool remove(const std::string& name, bool rec = false) {
			try {
				if (rec)	stdfs::remove_all(name); 
				else		stdfs::remove(name);
				return true;
			} catch (const stdfs::filesystem_error&) {
				return false;
			}
		}
		namespace cp {
			enum class type { 
				file,		  // copy file
				dir,		  // copy dir
				recdir,		  // rec copy dir
				link		  // copy link
			};

			inline bool
			cp(type t, const std::string& orig_name,
					const std::string& current_name)
			{
				try {
					switch(t) {
						case type::file:
							stdfs::copy(orig_name, current_name,
									stdfs::copy_options::overwrite_existing);
							break;
						case type::recdir:
							stdfs::copy(orig_name, current_name,
									stdfs::copy_options::recursive |
									stdfs::copy_options::overwrite_existing);
							break;
						case type::dir:
							stdfs::copy(orig_name, current_name);
							break;
						case type::link:
							stdfs::copy(orig_name, current_name, 
							stdfs::copy_options::copy_symlinks);
							break;
					}
					return true;
				} catch (const stdfs::filesystem_error&) {
					return false;
				}
			}
		} /* namespace cp */

		inline bool
		findstr(const std::string& file,
				const std::string& keyword,
				bool fm = false)
		{
			std::ifstream file_path(file);
			std::string line;
			if (!file_path.is_open()) return false;
			while (std::getline(file_path, line))
				if (fm ? (line.find(keyword) != std::string::npos) :
						 (line == keyword))
					return true;
			return false;
		}

		inline bool
		remove_str_from_file(const std::string& filename,
							const std::string& target)
		{
			std::ifstream infile(filename);
			if (!infile) return false;
			std::string content((std::istreambuf_iterator<char>(infile)),
								std::istreambuf_iterator<char>());
			infile.close();
			size_t pos = 0;
			while ((pos = content.find(target, pos)) != std::string::npos) content.erase(pos, target.length());

			std::ofstream outfile(filename);
			if (!outfile) return false;
			outfile << content;
			return true;
		}

		inline bool
		remove_text_line_from_file(const std::string& filename, const std::string& keyword)
		{
			std::ifstream infile(filename);
			if (!infile) return false;
			std::vector<std::string> lines;
			std::string line;
			while (std::getline(infile, line))
				if (line.find(keyword) == std::string::npos) lines.push_back(line);
			infile.close();
			std::ofstream outfile(filename);
			if (!outfile) return false;
			for (const auto& l : lines) outfile << l << '\n';
			return true;
		}

		inline bool __attribute__((deprecated))
		remove_file_str_or_remove_text_line(const std::string& filename,
											const std::string& target_or_keyword,
											bool def = true)
		{
			return def ? remove_str_from_file(filename, target_or_keyword)
						: remove_text_line_from_file(filename, target_or_keyword);
		}

		inline bool
		mklink(const std::string& target,
				const std::string& link_name,
				bool def = true)
		{
			try {
				if (def == true)	stdfs::create_symlink(target, link_name);
				else				stdfs::create_directory_symlink(target, link_name);
				return true;
			} catch (const stdfs::filesystem_error& e) {
				return false;
			}
		}

		namespace findtext {
			struct findresult {
				bool found = false;
				int line_number = 0;
				std::string line_content;
				int word_count = 0;
				findresult() = default;
				findresult(const std::string& file,
						const std::string& keyword,
						bool fm) { *this = xmz::fs::_::findtext::findresult(file, keyword, fm); }
			};

			inline findresult
			findtext(const std::string& file,
					const std::string& keyword,
					bool fm = false)
			{
				std::ifstream file_path(file);
				std::string line;
				findresult result;
				if (!file_path.is_open()) return result;
				while (std::getline(file_path, line)) {
					result.line_number++;
					bool match = false;
					if (!fm)	match = (line == keyword);
					else		match = (line.find(keyword) != std::string::npos);
					if (match) {
						result.found = true;
						result.line_content = line;
						result.word_count = 0;
						std::istringstream iss(line);
						std::string word;
						while (iss >> word) result.word_count++;
						return result;
					}
				}
				result.found = false;
				result.line_content = "";
				result.word_count = 0;
				return result;
			}
		} /* namespace findtext */
	} /* namespace fs::_ */
} /* namespace xmz */
#endif /* _LIBXMZ_FS_HPP */

