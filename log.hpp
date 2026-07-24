/*
 * log.hpp
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

#ifndef XMZ_TEAM_LOG_HPP
#define XMZ_TEAM_LOG_HPP

#include "io.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace xmz {
    namespace _log {
        /* color define */
        std::string XMZ_COLOR_NC = "\033[0m";
        std::string XMZ_COLOR_RED  = "\033[0;31m";
        std::string XMZ_COLOR_YELLOW = "\033[0;33m";
        std::string XMZ_COLOR_GREEN = "\033[0;32m";
        std::string XMZ_COLOR_GRAY = "\033[0;90m";
        std::string XMZ_COLOR_CYAN = "\033[0;36m";
        std::string XMZ_COLOR_TRBWC = "\033[5;37;41m"; // White characters on a red background(Twinkle)
       std::string XMZ_COLOR_RBYC = "\033[1;33;41m"; // Yellow characters on a red background
       std::string XMZ_COLOR_RBWC = "\033[1;37;41m";  // White characters on a red background
       /* log level macros */
       std::string  _EMERG_ = "[Emerg]:";
       std::string  _ALERT_ = "[Alert]:";
       std::string  _CRIT_ = "[Crit]:";
       std::string  _ERROR_ = "[Error]:";
       std::string  _WARN_ = "[Warn]:";
       std::string  _NOTICE_ = "[Notice]:";
       std::string  _INFO_ = "[Info]:";
       std::string  _DEBUG_ = "[Debug]:";
       std::string  _KEEP_ = "[Keep]:"; // no definition for the time being

        std::string emerg = XMZ_COLOR_TRBWC + _EMERG_ + XMZ_COLOR_NC;
        std::string alert = XMZ_COLOR_RBYC + _ALERT_ + XMZ_COLOR_NC;
        std::string crit = XMZ_COLOR_RBWC + _CRIT_ + XMZ_COLOR_NC;
        std::string error = XMZ_COLOR_RED + _ERROR_ + XMZ_COLOR_NC;
        std::string warn = XMZ_COLOR_YELLOW + _WARN_ + XMZ_COLOR_NC;
        std::string notice = XMZ_COLOR_CYAN + _NOTICE_ + XMZ_COLOR_NC;
        std::string info = XMZ_COLOR_GREEN + _INFO_ + XMZ_COLOR_NC;
        std::string debug = XMZ_COLOR_GRAY + _DEBUG_ + XMZ_COLOR_NC;

        inline std::string get_time_str() {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
               << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return ss.str();
        }
    } // namespace _log
    namespace log {
        inline std::string get_time() { return "[" + _log::get_time_str() + "]:"; }
        template<typename... Args>
        inline void emerg(const Args&... args) { xmz::perrln(xmz::_log::emerg, get_time(), args...); }
        template<typename... Args>
        inline void alert(const Args&... args) { xmz::perrln(xmz::_log::alert, get_time(), args...); }
        template<typename... Args>
        inline void crit(const Args&... args) { xmz::perrln(xmz::_log::crit, get_time(), args...); }
        template<typename... Args>
        inline void error(const Args&... args) { xmz::perrln(xmz::_log::error, get_time(), args...); }
        template<typename... Args>
        inline void warn(const Args&... args) { xmz::perrln(xmz::_log::warn, get_time(), args...); }
        template<typename... Args>
        inline void notice(const Args&... args) { xmz::perrln(xmz::_log::notice, get_time(), args...); }
        template<typename... Args>
        inline void info(const Args&... args) { xmz::println(xmz::_log::info, get_time(), args...); }
        template<typename... Args>
        inline void debug(const Args&... args) { xmz::perrln(xmz::_log::debug, get_time(), args...); }
        inline constexpr auto err = [](const std::string& msg) {
            error(msg);
        };
    } // namespace log
}

#endif // XMZ_TEAM_LOG_HPP
