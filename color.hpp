/*
 * color.hpp
 * Created by XMZ Studio and xhz <xhzer@outlook.com> on 2026-11-18
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

#ifndef LIBXMZ_COLOR_HPP
#define LIBXMZ_COLOR_HPP
#include <string>

namespace xmz::color {
    inline std::string NC		= "\033[0m";
    inline std::string RED		= "\033[0;31m";
    inline std::string YELLOW	= "\033[0;33m";
    inline std::string GREEN	= "\033[0;32m";
    inline std::string GRAY		= "\033[0;90m";
    inline std::string CYAN		= "\033[0;36m";
    inline std::string TRBWC	= "\033[5;37;41m";  // white characters on a red background(Twinkle)
    inline std::string RBYC		= "\033[1;33;41m";  // yellow characters on a red background
    inline std::string RBWC		= "\033[1;37;41m";  // white characters on a red background
} /* namespace xmz::color */
#endif /* LIBXMZ_COLOR_HPP */

