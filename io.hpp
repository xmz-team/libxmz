/*
 * io.hpp
 * Created by XMZ <xmz-team@outlook.com> on 6/7/26
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

#ifndef XMZ_TEAM_IO_HPP
#define XMZ_TEAM_IO_HPP
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <errno.h>
#include <type_traits>
#include <ostream>
#include <fstream>

namespace xmz {
    namespace io {
        namespace __io {
            inline int parse_fd(const char *fb) {
                if (fb == NULL) return -1;
                if (strcmp(fb, "stdout") == 0 || strcmp(fb, "1") == 0) return 1;
                if (strcmp(fb, "stderr") == 0 || strcmp(fb, "2") == 0) return 2;
                char *endptr;
                errno = 0;
                long val = strtol(fb, &endptr, 10);
                if (errno != 0 || *endptr != '\0' || endptr == fb) return -1;
                return (int)val;
            }

            inline int parse_fd(const std::string& fb) {
                if (fb.empty()) return -1;
                if (fb == "stdout" || fb == "1") return 1;
                if (fb == "stderr" || fb == "2") return 2;
                try {
                    return std::stoi(fb);
                } catch (...) {
                    return -1;
                }
            }

            inline ssize_t write_all(int fd, const void* buf, size_t count) {
                const char* ptr = static_cast<const char*>(buf);
                size_t remaining = count;
                while (remaining > 0) {
                    ssize_t written = write(fd, ptr, remaining);
                    if (written < 0) {
                        if (errno == EINTR) continue;
                        return -1;
                    }
                    remaining -= written;
                    ptr += written;
                }
                return count;
            }

            inline ssize_t write_string(int fd, const std::string& str) {
                return write_all(fd, str.data(), str.size());
            }

            inline ssize_t write_cstr(int fd, const char* str) {
                return write_all(fd, str, strlen(str));
            }
        } // namespace __io

        inline void print(const std::string& value) { __io::write_string(1, value); }
        inline void print(const char* value) { if (value) __io::write_cstr(1, value); }
        template<size_t N>
        inline void print(const char (&value)[N]) { __io::write_cstr(1, value); }
        inline void print(char* value) { if (value) __io::write_cstr(1, value); }
        // 处理布尔值
        inline void print(bool value) { __io::write_cstr(1, value ? "true" : "false"); }
        // 处理数值类型
        template<typename T>
        inline std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>
        print(const T& value) {
            __io::write_string(1, std::to_string(value));
        }
        // 变参模板
        template<typename T, typename... Args>
        inline void print(const T& first, const Args&... rest) {
            print(first);
            if constexpr (sizeof...(rest) > 0) { __io::write_all(1, " ", 1); }
            print(rest...);
        }

        template<typename... Args>
        inline void println(const Args&... args) {
            print(args...);
            __io::write_all(1, "\n", 1);
        }

        inline void perr(const std::string& value) { __io::write_string(2, value); }
        inline void perr(const char* value) { if (value) __io::write_cstr(2, value); }
        template<size_t N>
        inline void perr(const char (&value)[N]) { __io::write_cstr(2, value); }
        inline void perr(char* value) { if (value) __io::write_cstr(2, value); }
        // 处理布尔值
        inline void perr(bool value) { __io::write_cstr(2, value ? "true" : "false"); }
        // 处理数值类型
        template<typename T>
        inline std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>
        perr(const T& value) {
            __io::write_string(2, std::to_string(value));
        }
        // 变参模板
        template<typename T, typename... Args>
        inline void perr(const T& first, const Args&... rest) {
            perr(first);
            if constexpr (sizeof...(rest) > 0) { __io::write_all(2, " ", 1); }
            perr(rest...);
        }

        template<typename... Args>
        inline void perrln(const Args&... args) {
            print(args...);
            __io::write_all(2, "\n", 1);
        }

        inline ssize_t fprint(const char* fb, const char* text) {
            if (fb == nullptr || text == nullptr) return -1;
            int fd = __io::parse_fd(fb);
            if (fd < 0) return -1;
            return __io::write_cstr(fd, text);
        }

        inline ssize_t fprint(const std::string& fb, const std::string& text) {
            int fd = __io::parse_fd(fb);
            if (fd < 0) return -1;
            return __io::write_string(fd, text);
        }

        inline ssize_t fprintln(const char *fb, const char *text) {
            int fd = __io::parse_fd(fb);
            if (fd < 0) return -1;
            ssize_t ret = fprint(fb, text);
            if (ret < 0) return ret;
            ssize_t ret2 = __io::write_all(fd, "\n", 1);
            return (ret2 < 0) ? ret2 : ret + ret2;
        }

        inline ssize_t fprintln(const std::string& fb, const std::string& text) {
            int fd = __io::parse_fd(fb);
            if (fd < 0) return -1;
            ssize_t ret = fprint(fb, text);
            if (ret < 0) return ret;
            ssize_t ret2 = __io::write_all(fd, "\n", 1);
            return (ret2 < 0) ? ret2 : ret + ret2;
        }

        struct endl_t;
        struct _Setprecision;
        struct _Setw;
        struct fixed_t;
        struct scientific_t;
        struct hex_t;
        struct dec_t;
        struct oct_t;

        namespace fmt {
            enum flags {
                dec = 0x01,
                hex = 0x02,
                oct = 0x04,
                fixed = 0x08,
                scientific = 0x10,
                boolalpha = 0x20,
                showbase = 0x40,
                showpoint = 0x80,
                showpos = 0x100,
                uppercase = 0x200,
                left = 0x400,
                right = 0x800,
                internal = 0x1000,
            };
        }

        template<int Fd>
        struct out_stream {
            static constexpr int fd = Fd;
            fmt::flags flags_ = fmt::dec;
            int precision_ = 6;
            int width_ = 0;
            char fill_ = ' ';

            out_stream& setf(fmt::flags f) { flags_ = static_cast<fmt::flags>(flags_ | f); return *this; }
            out_stream& unsetf(fmt::flags f) { flags_ = static_cast<fmt::flags>(flags_ & ~f); return *this; }
            out_stream& precision(int p) { precision_ = p; return *this; }
            out_stream& width(int w) { width_ = w; return *this; }
            out_stream& fill(char c) { fill_ = c; return *this; }

            out_stream() = default;
            template<typename T>
            std::enable_if_t<
                !std::is_same_v<T, endl_t> &&
                !std::is_same_v<T, _Setprecision> &&
                !std::is_same_v<T, _Setw> &&
                !std::is_same_v<T, fixed_t> &&
                !std::is_same_v<T, scientific_t> &&
                !std::is_same_v<T, hex_t> &&
                !std::is_same_v<T, dec_t> &&
                !std::is_same_v<T, oct_t>,
                out_stream&
            >

            operator<<(const T& value) {
                if constexpr (std::is_same_v<T, std::string>) {
                    __io::write_string(Fd, value);
                } else if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>) {
                    __io::write_cstr(Fd, value);
                } else if constexpr (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>) {
                    __io::write_cstr(Fd, value);
                } else {
                    __io::write_string(Fd, std::to_string(value));
                }
                return *this;
            }
        };

        struct fixed_t {
            template<int Fd>
            friend out_stream<Fd>& operator<<(out_stream<Fd>& os, fixed_t) {
                os.setf(fmt::fixed);
                return os;
            }
        };
        inline constexpr fixed_t fixed;

        struct scientific_t {
            template<int Fd>
            friend out_stream<Fd>& operator<<(out_stream<Fd>& os, scientific_t) {
                os.setf(fmt::scientific);
                return os;
            }
        };
        inline constexpr scientific_t scientific;

        struct hex_t {
            template<int Fd>
            friend out_stream<Fd>& operator<<(out_stream<Fd>& os, hex_t) {
                os.setf(fmt::hex);
                return os;
            }
        };
        inline constexpr hex_t hex;

        struct dec_t {
            template<int Fd>
            friend out_stream<Fd>& operator<<(out_stream<Fd>& os, dec_t) {
                os.unsetf(fmt::hex | fmt::oct);
                os.setf(fmt::dec);
                return os;
            }
        };
        inline constexpr dec_t dec;

        struct oct_t {
            template<int Fd>
            friend out_stream<Fd>& operator<<(out_stream<Fd>& os, oct_t) {
                os.unsetf(fmt::hex | fmt::dec);
                os.setf(fmt::oct);
                return os;
            }
        };
        inline constexpr oct_t oct;

        struct _Setprecision { int n; };
        inline _Setprecision setprecision(int n) { return {n}; }
        template<int Fd>
        out_stream<Fd>& operator<<(out_stream<Fd>& os, const _Setprecision& sp) {
            os.precision(sp.n);
            return os;
        }

        struct _Setw { int n; };
        inline _Setw setw(int n) { return {n}; }
        template<int Fd>
        out_stream<Fd>& operator<<(out_stream<Fd>& os, const _Setw& sw) {
            os.width(sw.n);
            return os;
        }

        template<int Fd>
        inline out_stream<Fd>& flush(out_stream<Fd>& stream) {
            fsync(Fd);
            return stream;
        }

        struct endl_t {
            template<int Fd>
            friend out_stream<Fd>& operator<<(out_stream<Fd>& os, const endl_t&) {
                __io::write_all(Fd, "\n", 1);
                return flush(os);
            }
        };
        inline constexpr endl_t endl;

        inline out_stream<1> cout;
        inline out_stream<2> cerr;

    } // namespace io

    using io::print;
    using io::println;
    using io::perr;
    using io::perrln;
    using io::fprint;
    using io::fprintln;
    using io::cout;
    using io::cerr;
    using io::endl;
    using io::fixed;
    using io::scientific;
    using io::hex;
    using io::dec;
    using io::oct;
    using io::setprecision;
    using io::setw;
    inline auto echo = [](const auto& text) { println(text); };
} // namespace xmz

#endif // XMZ_TEAM_IO_HPP
