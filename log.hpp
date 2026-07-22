/*
 * log.hpp
 * Created by XMZ <xmz-team@outlook.com> on 10/3/26
 * Copyright (c) 2025-2026 XMZ <xmz-team@outlook.com> All rights reserved.
 */

#ifndef XMZ_TEAM_LOG_HPP
#define XMZ_TEAM_LOG_HPP

#include "io.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

/* color define */
#define XMZ_COLOR_NC "\033[0m"
#define XMZ_COLOR_RED "\033[0;31m"
#define XMZ_COLOR_YELLOW "\033[0;33m"
#define XMZ_COLOR_GREEN "\033[0;32m"
#define XMZ_COLOR_GRAY "\033[0;90m"
#define XMZ_COLOR_CYAN "\033[0;36m"
#define XMZ_COLOR_TRBWC "\033[5;37;41m" // White characters on a red background(Twinkle) - Emerg
#define XMZ_COLOR_RBYC "\033[1;33;41m" // Yellow characters on a red background - Alert
#define XMZ_COLOR_RBWC "\033[1;37;41m"  // White characters on a red background - Crit

/* log level macros */
#define _EMERG_ "[Emerg]: "
#define _ALERT_ "[Alert]: "
#define _CRIT_ "[Crit]: "
#define _ERROR_ "[Error]: "
#define _WARN_ "[Warn]: "
#define _NOTICE_ "[Notice]: "
#define _INFO_ "[Info]: "
#define _DEBUG_ "[Debug]: "
#define _KEEP_ "[Keep]: " // no definition for the time being

namespace xmz {
    namespace _log {
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
        template<typename... Args>
        inline void emerg(const Args&... args) { xmz::perrln(XMZ_COLOR_TRBWC, _EMERG_, XMZ_COLOR_NC, "[", _log::get_time_str(), "] ", args...); }
        template<typename... Args>
        inline void alert(const Args&... args) { xmz::perrln(XMZ_COLOR_RBYC, _ALERT_, XMZ_COLOR_NC, "[", _log::get_time_str(), "] ", args...); }
        template<typename... Args>
        inline void crit(const Args&... args) { xmz::perrln(XMZ_COLOR_RBWC, _CRIT_, XMZ_COLOR_NC, "[", _log::get_time_str(), "] ", args...); }
        template<typename... Args>
        inline void error(const Args&... args) { xmz::perrln(XMZ_COLOR_RED, _ERROR_, XMZ_COLOR_NC, "[", _log::get_time_str(), "] ", args...); }
        template<typename... Args>
        inline void warn(const Args&... args) { xmz::perrln(XMZ_COLOR_YELLOW, _WARN_, XMZ_COLOR_NC, "[", _log::get_time_str(), "] ", args...); }
        template<typename... Args>
        inline void notice(const Args&... args) { xmz::perrln(XMZ_COLOR_CYAN, _NOTICE_, XMZ_COLOR_NC, "[", _log::get_time_str(), "] ", args...); }
        template<typename... Args>
        inline void info(const Args&... args) { xmz::println(XMZ_COLOR_GREEN, _INFO_, XMZ_COLOR_NC, "[", _log::get_time_str(), "]: ", args...); }
        template<typename... Args>
        inline void debug(const Args&... args) { xmz::perrln(XMZ_COLOR_GRAY, _DEBUG_, XMZ_COLOR_NC, "[", _log::get_time_str(), "] ", args...); }

        inline constexpr auto err = [](const std::string& msg) {
            error(msg);
        };
    } // namespace log
}

#endif // XMZ_TEAM_LOG_HPP
