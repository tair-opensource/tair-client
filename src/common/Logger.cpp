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
#include "common/Logger.hpp"

#include "absl/debugging/stacktrace.h"
#include "absl/debugging/symbolize.h"
#include "spdlog/async.h"
#include "spdlog/details/fmt_helper.h"
#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"

#include "common/StringUtil.hpp"
#include "common/SystemUtil.hpp"

#define LOG_BACKGROUND_THREAD_COUNT 1

#define LOGGER_NAME        "default"
#define LOGGER_NAME_CHILD  "default-child"
#define MAX_LOG_FILE_SIZE  (100 * 1024 * 1024LU)
#define MAX_LOG_FILE_COUNT 5

#define DEFAULT_LOGGER_PATTERN  "[%Y-%m-%d %H:%M:%S.%e] [%L] [%t] (%s:%#) %v"
#define USERFLAG_LOGGER_PATTERN "[%Y-%m-%d %H:%M:%S.%e] [%*] [%L] [%t] (%s:%#) %v"

namespace tair::common {

class UserFormatterFlag : public spdlog::custom_flag_formatter {
public:
    void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override {
        static auto &logger = Logger::instance();
        auto &callback = logger.getUserFlagCallback();
        if (callback) {
            auto text = callback();
            dest.append(text.data(), text.data() + text.size());
        }
    }
    std::unique_ptr<custom_flag_formatter> clone() const override {
        return spdlog::details::make_unique<UserFormatterFlag>();
    }
};

void Logger::initLogger(const std::string &log_filename, bool dup_to_stdout, const UserFlagCallback &callback) {
    log_filename_ = log_filename;
    dup_to_stdout_ = dup_to_stdout;
    if (callback) {
        user_flag_callback_ = callback;
        pattern_ = USERFLAG_LOGGER_PATTERN;
    } else {
        pattern_ = DEFAULT_LOGGER_PATTERN;
    }

    // Init file sink and stdout sink
    std::vector<spdlog::sink_ptr> sinks;
    if (!log_filename.empty()) {
        auto async_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_filename, MAX_LOG_FILE_SIZE, MAX_LOG_FILE_COUNT);
        sinks.emplace_back(async_file_sink);
    }
    if (dup_to_stdout) {
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        sinks.emplace_back(stdout_sink);
    }

    // Init thread pool for async logger
    spdlog::init_thread_pool(spdlog::details::default_async_q_size, LOG_BACKGROUND_THREAD_COUNT,
                             []() { SystemUtil::setThreadName("spdlog-async"); });

    // Use nonblock mode, discard oldest message in the queue if full
    default_logger_ = spdlog::create_async_nb<spdlog::sinks::dist_sink_mt>(LOGGER_NAME, sinks);
    default_logger_->set_level(spdlog::level::trace);

#if !defined(NDEBUG)
    default_logger_->flush_on(spdlog::level::trace);
#else
    default_logger_->flush_on(spdlog::level::info);
#endif

    if (user_flag_callback_) {
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<UserFormatterFlag>('*').set_pattern(pattern_);
        default_logger_->set_formatter(std::move(formatter));
    } else {
        default_logger_->set_pattern(pattern_);
    }

    default_logger_->set_error_handler([](const std::string &err) {
        fprintf(stderr, "some error in logger: %s\ncall stack:\n", err.c_str());
        void *results[10];
        char frame[128];
        int ret = absl::GetStackTrace(results, 10, 1);
        for (int i = 0; i < ret; ++i) {
            if (absl::Symbolize(results[i], frame, sizeof(frame))) {
                fprintf(stderr, "\t%s\n", frame);
            }
        }
    });
    spdlog::set_default_logger(default_logger_);

    // Init for child process logger
    initAfterFork();
}

void Logger::flushAllLogAndWait(int max_wait_ms) {
    auto &registry = spdlog::details::registry::instance();
    registry.set_level(spdlog::level::off);
    registry.flush_all();
    auto logger_thread_pool = registry.get_tp();
    int sleep_ms = 0;
    while (logger_thread_pool && logger_thread_pool->queue_size() != 0 && sleep_ms < max_wait_ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        sleep_ms++;
    }
}

void Logger::setLogLevel(const std::string &level) {
    if (default_logger_) {
        if (level == "TRACE") {
            default_logger_->set_level(spdlog::level::trace);
        } else if (level == "DEBUG") {
            default_logger_->set_level(spdlog::level::debug);
        } else if (level == "INFO") {
            default_logger_->set_level(spdlog::level::info);
        } else if (level == "WARN") {
            default_logger_->set_level(spdlog::level::warn);
        } else if (level == "ERROR") {
            default_logger_->set_level(spdlog::level::err);
        }
    }
}

const UserFlagCallback &Logger::getUserFlagCallback() const {
    return user_flag_callback_;
}

void Logger::initAfterFork() {
    ::pthread_atfork(nullptr, nullptr, []() {
        auto &logger = Logger::instance();
        std::vector<spdlog::sink_ptr> sinks;
        if (!logger.log_filename_.empty()) {
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(logger.log_filename_);
            sinks.emplace_back(file_sink);
        }
        if (logger.dup_to_stdout_) {
            auto stdout_sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
            sinks.emplace_back(stdout_sink);
        }
        auto new_logger = spdlog::create<spdlog::sinks::dist_sink_st>(LOGGER_NAME_CHILD, sinks);
#if !defined(NDEBUG)
        new_logger->flush_on(spdlog::level::trace);
#else
    new_logger->flush_on(spdlog::level::info);
#endif
        new_logger->set_level(spdlog::level::trace);

        if (logger.user_flag_callback_) {
            auto formatter = std::make_unique<spdlog::pattern_formatter>();
            formatter->add_flag<UserFormatterFlag>('*').set_pattern(logger.pattern_);
            new_logger->set_formatter(std::move(formatter));
        } else {
            new_logger->set_pattern(logger.pattern_);
        }

        new_logger->set_error_handler([](const std::string &err) {
            fprintf(stderr, "some error in logger: %s\ncall stack:\n", err.c_str());
            void *results[10];
            char frame[128];
            int ret = absl::GetStackTrace(results, 10, 1);
            for (int i = 0; i < ret; ++i) {
                if (absl::Symbolize(results[i], frame, sizeof(frame))) {
                    fprintf(stderr, "\t%s\n", frame);
                }
            }
        });
        spdlog::set_default_logger(new_logger);
    });
}

class PayloadLogFormatter : public spdlog::formatter {
public:
    void format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) override {
        spdlog::details::fmt_helper::append_string_view(msg.payload, dest);
    }
    std::unique_ptr<formatter> clone() const override {
        return spdlog::details::make_unique<PayloadLogFormatter>();
    }
};

LoggerPtr Logger::getPayloadLogger(const std::string &logger_name, const std::string &filename,
                                   uint64_t file_size, uint64_t file_num) {
    try {
        auto logger = spdlog::rotating_logger_st(logger_name, filename, file_size, file_num);
        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::info);
        logger->set_formatter(std::make_unique<PayloadLogFormatter>());
        return logger;
    } catch (const spdlog::spdlog_ex &ex) {
        LOG_ERROR("get logger {} to file {} failed: {}", logger_name, filename, ex.what());
        Logger::instance().flushAllLogAndWait();
        ::abort();
    }
    return nullptr;
}

} // namespace tair::common
