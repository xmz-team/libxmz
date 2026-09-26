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
#include <string>
#include <errno.h>
#include <system_error>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <array>
#include <atomic>
#include <vector>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <chrono>
#include <type_traits>

#if __cplusplus >= 202002L && __has_include(<format>)
#    include <format>
#    ifdef __cpp_lib_format
#        define _XMZ_HAS_STD_FORMAT 1
#    else
#        define _XMZ_HAS_STD_FORMAT 0
#        include <sstream>
#        include <iomanip>
#    endif
#else
#    define _XMZ_HAS_STD_FORMAT 0
#    include <sstream>
#    include <iomanip>
#endif

namespace xmz {
    namespace io {
        class io_error : public std::system_error {
        public:
            explicit io_error(const std::string& message)
                : std::system_error(errno, std::generic_category(), message) {}
            explicit io_error(int error_code, const std::string& message)
                : std::system_error(error_code, std::generic_category(), message) {}
        };

        static constexpr size_t RING_BUFFER_SIZE = 4096;
        static constexpr int DEFAULT_FLUSH_TIMEOUT_MS = 100;
        static constexpr int MAX_FLUSH_RETRIES = 10;

        template<typename T, size_t N>
        class ring_buffer {
        public:
            ring_buffer() : head(0), tail(0) {}
            bool push(const T& item) {
                size_t next_tail = (tail + 1) % N;
                if (next_tail == head) return false;
                buffer[tail] = item;
                tail = next_tail;
                return true;
            }

            bool pop(T& item) {
                if (head == tail) return false;
                item = buffer[head];
                head = (head + 1) % N;
                return true;
            }

            size_t size() const { return (tail - head + N) % N; }
            bool empty() const { return head == tail; }
            bool full() const { return (tail + 1) % N == head; }
            size_t capacity() const { return N; }
            size_t available() const { return (tail >= head) ? (N - (tail - head) - 1) : (head - tail - 1); }

            size_t peek(char* dest, size_t max_count) const {
                if (empty() || dest == nullptr) return 0;
                size_t count = 0;
                size_t idx = head;
                while (count < max_count && idx != tail) {
                    dest[count] = buffer[idx];
                    count++;
                    idx = (idx + 1) % N;
                }
                return count;
            }

            size_t pop_bulk(char* dest, size_t max_count) {
                if (empty() || max_count == 0) return 0;
                size_t count = 0;
                size_t idx = head;
                if (dest != nullptr) {
                    while (count < max_count && idx != tail) {
                        dest[count] = buffer[idx];
                        count++;
                        idx = (idx + 1) % N;
                    }
                } else {
                    while (count < max_count && idx != tail) {
                        count++;
                        idx = (idx + 1) % N;
                    }
                }
                head = idx;
                return count;
            }
        private:
            std::array<T, N> buffer;
            size_t head;
            size_t tail;
        };

        class fd_buffer : public std::enable_shared_from_this<fd_buffer> {
        public:
            explicit fd_buffer(int f) : fd(f), closed(false), use_count(1) {
                if (fd < 0) throw io_error("Invalid file descriptor");
                int flags = ::fcntl(fd, F_GETFL, 0);
                if (flags == -1) throw io_error("Failed to get fd flags");
                if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
                    throw io_error("Failed to set non-blocking mode");
                }
            }

            ~fd_buffer() {
                try { flush_force(); } catch (...) {}
                if (fd >= 0 && !closed) { ::close(fd); }
            }

            fd_buffer(const fd_buffer&) = delete;
            fd_buffer& operator=(const fd_buffer&) = delete;

            bool flush(int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) {
                std::lock_guard<std::mutex> lock(mtx);
                return flush_internal(timeout_ms);
            }

            void flush_force(int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) {
                std::lock_guard<std::mutex> lock(mtx);
                if (!flush_internal(timeout_ms)) {
                    throw io_error("Failed to force flush: timeout exceeded");
                }
            }

            bool write_all(const void* buf, size_t count, int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) {
                if (count == 0) return true;
                if (closed) throw io_error("File descriptor is closed");
                if (buf == nullptr) throw io_error("Null buffer pointer");
                const char* ptr = static_cast<const char*>(buf);
                size_t remaining = count;
                while (remaining > 0) {
                    size_t to_copy = std::min(remaining, buffer.available());
                    if (to_copy > 0) {
                        for (size_t i = 0; i < to_copy; ++i) { buffer.push(ptr[i]); }
                        remaining -= to_copy;
                        ptr += to_copy;
                    }

                    if (buffer.size() > 0) { if (!flush_internal(timeout_ms)) { return false; } }
                }
                return true;
            }

            void write_string(const std::string& str, int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) { 
                write_all(str.data(), str.size(), timeout_ms); 
            }

            void write_cstr(const char* str, int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) {
                if (!str) throw io_error("Null string pointer");
                write_all(str, strlen(str), timeout_ms);
            }

            void close_fd() {
                std::lock_guard<std::mutex> lock(mtx);
                if (closed) return;
                flush_force();
                if (::close(fd) < 0) { throw io_error("Failed to close file descriptor"); }
                closed = true;
            }

            bool is_closed() const { return closed; }
            int get_fd() const { return fd; }
            size_t buffer_size() const { return buffer.size(); }
            bool buffer_empty() const { return buffer.empty(); }

            void add_ref() { use_count.fetch_add(1, std::memory_order_relaxed); }
            void release_ref() { 
                if (use_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    delete this;
                }
            }
        private:
            int fd;
            ring_buffer<char, RING_BUFFER_SIZE> buffer;
            bool closed;
            std::mutex mtx;
            std::atomic<int> use_count;

            bool wait_for_write(int timeout_ms) {
                fd_set writefds;
                struct timeval tv;
                int ret;
                do {
                    FD_ZERO(&writefds);
                    FD_SET(fd, &writefds);
                    tv.tv_sec = timeout_ms / 1000;
                    tv.tv_usec = (timeout_ms % 1000) * 1000;
                    ret = ::select(fd + 1, nullptr, &writefds, nullptr, timeout_ms >= 0 ? &tv : nullptr);
                } while (ret < 0 && errno == EINTR);
                if (ret < 0) { throw io_error("select error while waiting for write"); }
                return ret > 0;
            }

            bool flush_internal(int timeout_ms) {
    std::array<char, RING_BUFFER_SIZE> temp_buffer;
            int retry_count = 0;
            auto start_time = std::chrono::steady_clock::now();
            while (!buffer.empty()) {
                if (timeout_ms >= 0) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start_time).count();
                    if (elapsed >= timeout_ms) return false;
                }
                size_t count = buffer.peek(temp_buffer.data(), temp_buffer.size());
                if (count == 0) break;
                ssize_t written = ::write(fd, temp_buffer.data(), count);
                if (written < 0) {
                    if (errno == EINTR) continue;
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        int remaining_timeout = timeout_ms >= 0 ? 
                            timeout_ms - std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - start_time).count() : -1;
                        if (remaining_timeout < 0 && timeout_ms >= 0) return false;
                        if (!wait_for_write(remaining_timeout >= 0 ? remaining_timeout : 100)) {
                            retry_count++;
                            if (retry_count >= MAX_FLUSH_RETRIES && timeout_ms >= 0) return false;
                            continue;
                        }
                        retry_count = 0;
                        continue;
                    }
                    throw io_error("Failed to write to file descriptor");
                }
                if (written > 0) {
                    size_t popped = buffer.pop_bulk(nullptr, static_cast<size_t>(written));
                    // char discard[64];
                    // size_t popped = buffer.pop_bulk(discard, written);
                    }
                    retry_count = 0;
                }
                return true;
            }
        };

        class buffer_manager {
        public:
            static buffer_manager& instance() {
                static buffer_manager inst;
                return inst;
            }

            std::shared_ptr<fd_buffer> get_buffer(int fd) {
                std::lock_guard<std::mutex> lock(mtx);
                auto it = buffers.find(fd);
                if (it != buffers.end() && !it->second->is_closed()) { return it->second; }
                auto buf = std::make_shared<fd_buffer>(fd);
                buffers[fd] = buf;
                return buf;
            }

            bool flush_fd(int fd, int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) {
                std::lock_guard<std::mutex> lock(mtx);
                auto it = buffers.find(fd);
                if (it != buffers.end() && !it->second->is_closed()) { return it->second->flush(timeout_ms); }
                return true;
            }

            void close_fd(int fd) {
                std::lock_guard<std::mutex> lock(mtx);
                auto it = buffers.find(fd);
                if (it != buffers.end() && !it->second->is_closed()) {
                    it->second->close_fd();
                    buffers.erase(it);
                }
            }

            void flush_all(int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) {
                std::lock_guard<std::mutex> lock(mtx);
                for (auto& pair : buffers) { if (!pair.second->is_closed()) { try { pair.second->flush(timeout_ms); } catch (...) {} } }
            }

            ~buffer_manager() { flush_all(); }

        private:
            buffer_manager() = default;
            buffer_manager(const buffer_manager&) = delete;
            buffer_manager& operator=(const buffer_manager&) = delete;
            std::mutex mtx;
            std::unordered_map<int, std::shared_ptr<fd_buffer>> buffers;
        };
#if !_XMZ_HAS_STD_FORMAT
        template<typename T>
        inline std::string format_fallback(const T& value) {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
        template<>
        inline std::string format_fallback<std::string>(const std::string& value) {
            return value;
        }
        template<>
        inline std::string format_fallback<const char*>(const char* const& value) {
            return value ? std::string(value) : "(null)";
        }
#endif
        template<typename T>
        inline std::string format_value(const T& value) {
            if constexpr (std::is_same_v<T, char>) {
                return std::string(1, value);
            } else if constexpr (std::is_same_v<T, const char*>) {
                return std::string(value ? value : "(null)");
            } else if constexpr (std::is_same_v<T, char*>) {
                return std::string(value ? value : "(null)");
            } else if constexpr (std::is_same_v<T, bool>) {
                return value ? "true" : "false";
            } else if constexpr (std::is_arithmetic_v<T>) {
                return std::to_string(value);
            } else {
#if XMZ_HAS_STD_FORMAT
                return std::format("{}", value);
#else
                return format_fallback(value);
#endif
            }
        }

        template<typename T>
        struct is_string_like : std::false_type {};
        template<>
        struct is_string_like<std::string> : std::true_type {};
        template<>
        struct is_string_like<const char*> : std::true_type {};
        template<>
        struct is_string_like<char*> : std::true_type {};

        template<size_t N>
        struct is_string_like<const char[N]> : std::true_type {};

        template<size_t N>
        struct is_string_like<char[N]> : std::true_type {};

        inline std::shared_ptr<fd_buffer> get_buffer(int fd) { return buffer_manager::instance().get_buffer(fd); }
        inline void flush_all(int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) { buffer_manager::instance().flush_all(timeout_ms); }
        inline bool flush(int fd, int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) { return buffer_manager::instance().flush_fd(fd, timeout_ms); }
        inline void close_fd(int fd) { buffer_manager::instance().close_fd(fd); }
        inline bool write_all(int fd, const void* buf, size_t count, 
                              int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) { return get_buffer(fd)->write_all(buf, count, timeout_ms); }
        inline void write_string(int fd, const std::string& str, 
                                 int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) { get_buffer(fd)->write_string(str, timeout_ms); }
        inline void write_cstr(int fd, const char* str, 
                               int timeout_ms = DEFAULT_FLUSH_TIMEOUT_MS) { 
            get_buffer(fd)->write_cstr(str, timeout_ms); 
        }

        template<typename T>
        void print(const T& value) {
            if constexpr (is_string_like<T>::value) {
                if constexpr (std::is_pointer_v<T>) { if (!value) throw io_error("Null string pointer"); }
                write_string(1, format_value(value));
            } else {
                write_string(1, format_value(value));
            }
        }

        template<typename T, typename... Args>
        void print(const T& first, const Args&... rest) {
            print(first);
            if constexpr (sizeof...(rest) > 0) { write_all(1, " ", 1); }
            print(rest...);
        }

        template<typename... Args>
        void println(const Args&... args) {
            print(args...);
            write_all(1, "\n", 1);
        }

        template<typename T>
        void perr(const T& value) {
            if constexpr (is_string_like<T>::value) {
                if constexpr (std::is_pointer_v<T>) { if (!value) throw io_error("Null string pointer"); }
                write_string(2, format_value(value));
            } else {
                write_string(2, format_value(value));
            }
        }

        template<typename T, typename... Args>
        void perr(const T& first, const Args&... rest) {
            perr(first);
            if constexpr (sizeof...(rest) > 0) { write_all(2, " ", 1); }
            perr(rest...);
        }

        template<typename... Args>
        void perrln(const Args&... args) {
            perr(args...);
            write_all(2, "\n", 1);
        }

        template<typename T>
        void fprint(int fd, const T& value) {
            if (fd < 0) throw io_error("Invalid file descriptor");
            if constexpr (is_string_like<T>::value) {
                if constexpr (std::is_pointer_v<T>) { if (!value) throw io_error("Null string pointer"); }
                write_string(fd, format_value(value));
            } else {
                write_string(fd, format_value(value));
            }
        }

        template<typename T, typename... Args>
        void fprint(int fd, const T& first, const Args&... rest) {
            fprint(fd, first);
            if constexpr (sizeof...(rest) > 0) { write_all(fd, " ", 1); }
            fprint(fd, rest...);
        }

        template<typename... Args>
        void fprintln(int fd, const Args&... args) {
            fprint(fd, args...);
            write_all(fd, "\n", 1);
        }

        class scoped_fd {
        public:
            explicit scoped_fd(int f) : fd(f) { if (fd < 0) throw io_error("Invalid file descriptor"); }

            ~scoped_fd() { try { if (fd >= 0) close_fd(fd); } catch (...) {} }

            scoped_fd(const scoped_fd&) = delete;
            scoped_fd& operator=(const scoped_fd&) = delete;
            scoped_fd(scoped_fd&& other) noexcept : fd(other.fd) { 
                other.fd = -1; 
            }

            scoped_fd& operator=(scoped_fd&& other) noexcept {
                if (this != &other) {
                    if (fd >= 0) close_fd(fd);
                    fd = other.fd;
                    other.fd = -1;
                }
                return *this;
            }

            int get() const { return fd; }
            operator int() const { return fd; }

            void reset(int new_fd = -1) {
                if (fd >= 0) close_fd(fd);
                fd = new_fd;
                if (fd < 0) throw io_error("Invalid file descriptor");
            }
        private:
            int fd;
        };
    } // namespace io
    using namespace io;
} // namespace xmz

#endif // XMZ_TEAM_IO_HPP
