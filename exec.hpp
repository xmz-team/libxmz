/*
 * exec.hpp
 * Created by XMZ <xmz-team@outlook.com> on 12/7/26
 * Copyright (c) 2026 XMZ <xmz-team@outlook.com> All rights reserved.
 */
/*
because Apple has blocked execvp and execv, you can only use execve to implement execv and execvp.
this is not rebuilding the wheel...
  */

#ifndef XMZ_TEAM_EXEC_HPP
#define XMZ_TEAM_EXEC_HPP

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>

#ifndef _PATH_DEFPATH
// #define _PATH_DEFPATH  "/usr/bin:/bin:/usr/sbin:/sbin" // origin
#define _PATH_DEFPATH  "/usr/bin:/bin:/usr/sbin:/sbin:/var/jb/usr/bin:/var/jb/bin:/var/jb/sbin:/var/jb/usr/sbin:/var/jb/usr/local/bin:/var/jb/usr/local/sbin:/rootfs/bin:/rootfs/sbin:/rootfs/usr/bin:/rootfs/usr/sbin" // rootless, rootful(or system), roothide
#endif

extern char **environ;

namespace xmz::_exec {
    // Types
    struct proc { char **p_envp; };
    typedef struct proc *proc_t;
    struct execve_args {
        const char *fname;
        char * const *argp;
        char * const *envp;
    };
    struct execv_args {
        const char *path;
        char * const *argv;
    };
    struct execvp_args {
        const char *file;
        char * const *argv;
    };
    // RAII Path Buffer
    class path_buffer {
    public:
        explicit path_buffer(size_t size = MAXPATHLEN)
            : size_(size), ptr_(nullptr) {
            ptr_ = static_cast<char*>(malloc(size));
            if (!ptr_) {
                throw std::bad_alloc();
            }
        }
        ~path_buffer() { if (ptr_) { free(ptr_); } }
        path_buffer(const path_buffer&) = delete;
        path_buffer& operator=(const path_buffer&) = delete;
        path_buffer(path_buffer&& other) noexcept
            : size_(other.size_), ptr_(other.ptr_) {
            other.ptr_ = nullptr;
            other.size_ = 0;
        }
        path_buffer& operator=(path_buffer&& other) noexcept {
            if (this != &other) {
                if (ptr_) { free(ptr_); }
                ptr_ = other.ptr_;
                size_ = other.size_;
                other.ptr_ = nullptr;
                other.size_ = 0;
            }
            return *this;
        }
        char* get() { return ptr_; }
        const char* get() const { return ptr_; }
        operator char*() { return ptr_; }
        operator const char*() const { return ptr_; }
        size_t size() const { return size_; }
        bool valid() const { return ptr_ != nullptr; }
    private:
        size_t size_;
        char* ptr_;
    };
    // RAII EM(Environment Manager)
    class env_manager {
    public:
        env_manager() : envp_(environ) {}
        explicit env_manager(char** envp) : envp_(envp) {}
        char** get() const { return envp_; }
        operator char**() const { return envp_; }
        char* getenv(const char* name) const {
            if (!envp_) return nullptr;
            size_t namelen = strlen(name);
            for (char** env = envp_; *env; ++env) { if (strncmp(*env, name, namelen) == 0 && (*env)[namelen] == '=') { return *env + namelen + 1; } }
            return nullptr;
        }
    private:
        char** envp_;
    };
    // RAII AB(Argument Builder)
    class arg_builder {
    public:
        arg_builder() = default;
        template<typename... Args>
        arg_builder(const char* first, Args... rest) {
            add(first);
            (add(rest), ...);
        }
        arg_builder& add(const char* arg) {
            args_.push_back(arg);
            return *this;
        }
        arg_builder& add(const std::string& arg) {
            args_.push_back(arg.c_str());
            return *this;
        }
        char* const* build() {
            mutable_args_.clear();
            for (auto arg : args_) {
                mutable_args_.push_back(const_cast<char*>(arg));
            }
            mutable_args_.push_back(nullptr);
            return mutable_args_.data();
        }
        size_t size() const { return args_.size(); }
        void clear() { args_.clear(); mutable_args_.clear(); }
    private:
        std::vector<const char*> args_;
        std::vector<char*> mutable_args_;
    };
    // Helper functions
    static inline char* getenv_from_proc(proc_t p, const char* name) {
        if (p != NULL && p->p_envp != NULL) {
            char** env = p->p_envp;
            size_t namelen = strlen(name);
            while (*env) {
                if (strncmp(*env, name, namelen) == 0 && (*env)[namelen] == '=') {
                    return *env + namelen + 1;
                }
                env++;
            }
            return NULL;
        }
        return getenv(name);
    }
    static inline bool file_is_executable(const char* path) {
        struct stat st;
        if (stat(path, &st) != 0)
            return false;
        return (S_ISREG(st.st_mode) && (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)));
    }
    static inline bool file_is_executable(const std::string& path) {
        return file_is_executable(path.c_str());
    }
} // namespace xmz::_exec
namespace xmz::exec {
    // PATH search implementation
    static int
    search_path(_exec::proc_t p, const char* filename, char* pathbuf, size_t buflen) {
        char* path = _exec::getenv_from_proc(p, "PATH");
        if (path == NULL)
            path = (char*)_PATH_DEFPATH;
        char *cp, *start;
        for (start = path; *start != '\0'; start = cp) {
            for (cp = start; *cp != '\0' && *cp != ':'; cp++)
                ;
            size_t len = cp - start;
            if (len == 0) {
                if (*cp != '\0') cp++;
                continue;
            }
            if (len + 1 + strlen(filename) + 1 > buflen)
                continue;
            memcpy(pathbuf, start, len);
            pathbuf[len] = '/';
            strcpy(pathbuf + len + 1, filename);
            if (_exec::file_is_executable(pathbuf)) { return 0; }
            if (*cp != '\0') cp++;
        }
        return ENOENT;
    }
    static inline int
    execv(_exec::proc_t p, struct _exec::execv_args* uap, int32_t* retval __unused) {
        struct _exec::execve_args eap;
        eap.fname = uap->path;
        eap.argp = uap->argv;
        _exec::env_manager env(p ? p->p_envp : environ);
        eap.envp = env.get();
        return ::execve(eap.fname, eap.argp, eap.envp);
    }
    static inline int
    execvp(_exec::proc_t p, struct _exec::execvp_args* uap, int32_t* retval __unused) {
        struct _exec::execve_args eap;
        _exec::env_manager env(p ? p->p_envp : environ);
        // Check if file contains '/'
        bool has_slash = (strchr(uap->file, '/') != nullptr);
        if (has_slash) {
            eap.fname = uap->file;
            eap.argp = uap->argv;
            eap.envp = env.get();
            return ::execve(eap.fname, eap.argp, eap.envp);
        }
        // Search PATH
        _exec::path_buffer buf;
        int error = search_path(p, uap->file, buf, buf.size());
        if (error) { return error; }
        eap.fname = buf.get();
        eap.argp = uap->argv;
        eap.envp = env.get();
        return ::execve(eap.fname, eap.argp, eap.envp);
    }
    template<typename... Args>
    static inline int execv(_exec::proc_t p, const char* path, Args... args) {
        _exec::arg_builder builder(path, args...);
        struct _exec::execv_args uap;
        uap.path = path;
        uap.argv = builder.build();
        int32_t retval;
        return execv(p, &uap, &retval);
    }
    template<typename... Args>
    static inline int execvp(_exec::proc_t p, const char* file, Args... args) {
        _exec::arg_builder builder(file, args...);
        struct _exec::execvp_args uap;
        uap.file = file;
        uap.argv = builder.build();
        int32_t retval;
        return execvp(p, &uap, &retval);
    }
    static inline int execv(_exec::proc_t p, const std::string& path, char* const* argv) {
        struct _exec::execv_args uap;
        uap.path = path.c_str();
        uap.argv = argv;
        int32_t retval;
        return execv(p, &uap, &retval);
    }
    static inline int execvp(_exec::proc_t p, const std::string& file, char* const* argv) {
        struct _exec::execvp_args uap;
        uap.file = file.c_str();
        uap.argv = argv;
        int32_t retval;
        return execvp(p, &uap, &retval);
    }
    static inline int execvp(_exec::proc_t p, const std::string& file,
                             const std::vector<std::string>& args) {
        _exec::arg_builder builder;
        builder.add(file);
        for (const auto& arg : args) { builder.add(arg); }
        struct _exec::execvp_args uap;
        uap.file = file.c_str();
        uap.argv = builder.build();
        int32_t retval;
        return execvp(p, &uap, &retval);
    }
    // Utilities
    static inline std::string search_path(_exec::proc_t p, const std::string& filename) {
        _exec::path_buffer buf;
        int result = search_path(p, filename.c_str(), buf, buf.size());
        if (result == 0) { return std::string(buf.get()); }
        return std::string();
    }
    static inline bool file_is_executable(const char* path) { return _exec::file_is_executable(path); }
    static inline bool file_is_executable(const std::string& path) { return _exec::file_is_executable(path); }
} // namespace xmz::exec
#endif /* XMZ_TEAM_EXEC_HPP */
