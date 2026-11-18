/*
 * log.hpp
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

#ifndef LIBXMZ_LOG_HPP
#define LIBXMZ_LOG_HPP
#include "io.hpp"
#include "time.hpp"
#include "color.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace xmz {
    namespace log {
		namespace lv {
			/* log level macros */
			inline std::string  EMERG	= "[Emerg]:";
			inline std::string  ALERT	= "[Alert]:";
			inline std::string  CRIT	= "[Crit]:";
			inline std::string  ERROR	= "[Error]:";
			inline std::string  WARN	= "[Warn]:";
			inline std::string  NOTICE	= "[Notice]:";
			inline std::string  INFO	= "[Info]:";
			inline std::string  DEBUG	= "[Debug]:";
			inline std::string  KEEP	= "[Keep]:"; // no definition for the time being

			inline std::string emerg	= xmz::color::TRBWC + EMERG + xmz::color::NC;
			inline std::string alert	= xmz::color::RBYC + ALERT + xmz::color::NC;
			inline std::string crit		= xmz::color::RBWC + CRIT + xmz::color::NC;
			inline std::string error	= xmz::color::RED + ERROR + xmz::color::NC;
			inline std::string warn		= xmz::color::YELLOW + WARN + xmz::color::NC;
			inline std::string notice	= xmz::color::CYAN + NOTICE + xmz::color::NC;
			inline std::string info		= xmz::color::GREEN + INFO + xmz::color::NC;
			inline std::string debug	= xmz::color::GRAY + DEBUG + xmz::color::NC;
        } /* namespace lv */
        inline std::string get_time() { return "[" + xmz::get_time_str() + "]:"; }

        template<typename... Args>
        inline void emerg(const Args&... args)
		{ xmz::perrln(xmz::log::lv::emerg, get_time(), args...); }

        template<typename... Args>
        inline void alert(const Args&... args)
		{ xmz::perrln(xmz::log::lv::alert, get_time(), args...); }

        template<typename... Args>
        inline void crit(const Args&... args)
		{ xmz::perrln(xmz::log::lv::crit, get_time(), args...); }

		template<typename... Args>
        inline void error(const Args&... args)
		{ xmz::perrln(xmz::log::lv::error, get_time(), args...); }

		template<typename... Args>
        inline void warn(const Args&... args)
		{ xmz::perrln(xmz::log::lv::warn, get_time(), args...); }

		template<typename... Args>
        inline void notice(const Args&... args)
		{ xmz::perrln(xmz::log::lv::notice, get_time(), args...); }

		template<typename... Args>
        inline void info(const Args&... args)
		{ xmz::println(xmz::log::lv::info, get_time(), args...); }

		template<typename... Args>
        inline void debug(const Args&... args)
		{ xmz::perrln(xmz::log::lv::debug, get_time(), args...); }

		inline constexpr auto err = [](const std::string& msg) { error(msg); };

		template<typename... Args>
        inline void custom(const std::string& info_type, const Args&... args)
		{ xmz::println("[" + info_type + "]", args...); }
    } /* namespace log */
}
#endif /* LIBXMZ_LOG_HPP */

