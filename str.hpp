/*
 * str.hpp
 * Created by XMZ <xmz-team@outlook.com> on 23/7/26
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

#ifndef XMZ_TEAM_STR_HPP
#define XMZ_TEAM_STR_HPP

#include <string>
#include <vector>

namespace xmz {
    namespace str {
        inline std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
            std::vector<std::string> tokens;
            size_t start = 0, end;
            while ((end = s.find(delimiter, start)) != std::string::npos) {
                tokens.push_back(s.substr(start, end - start));
                start = end + delimiter.length();
            }
            tokens.push_back(s.substr(start));
            return tokens;
        }
        inline std::string trim(const std::string& s) {
            auto start = s.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) return "";
            auto end = s.find_last_not_of(" \t\n\r");
            return s.substr(start, end - start + 1);
        }
    } /* namespace str */
} /* namespace xmz */
#endif /* XMZ_TEAM_STR_HPP */
