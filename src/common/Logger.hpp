/*
 *  Copyright (c) 2023 Tair
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#pragma once

// __PRETTY_FUNCTION__ is nicer in clang/gcc, and can avoid warnings in lambda functions.
#define SPDLOG_FUNCTION __PRETTY_FUNCTION__

#include "fmt/std.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

#include "common/Noncopyable.hpp"

#define LOG_TRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)     SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...)     SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...)    SPDLOG_ERROR(__VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

#define DEFAULT_LOGGER_FILE_SIZE (100 * 1024 * 1024) // 100MB
#define DEFAULT_LOGGER_FILE_NUM  10

namespace tair::common {

using UserFlagCallback = std::function<std::string_view(void)>;

using LoggerPtr = std::shared_ptr<spdlog::logger>;

class Logger final : private Noncopyable {
public:
    static Logger &instance() {
        static Logger logger;
        return logger;
    }

private:
    Logger() = default;

public:
    ~Logger() = default;

    void initLogger(const std::string &log_filename, bool dup_to_stdout = false,
                    const UserFlagCallback &callback = UserFlagCallback());
    void flushAllLogAndWait(int max_wait_ms = 1000);

    void setLogLevel(const std::string &level);

    const UserFlagCallback &getUserFlagCallback() const;

    static LoggerPtr getPayloadLogger(const std::string &logger_name, const std::string &filename,
                                      uint64_t file_size = DEFAULT_LOGGER_FILE_SIZE,
                                      uint64_t file_num = DEFAULT_LOGGER_FILE_NUM);

private:
    static void initAfterFork();

private:
    std::string log_filename_;
    bool dup_to_stdout_ = false;
    std::string pattern_;
    LoggerPtr default_logger_;
    UserFlagCallback user_flag_callback_;
};

} // namespace tair::common
