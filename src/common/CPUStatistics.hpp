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

#include <cstdint>
#include <sstream>

#include "common/ClockTime.hpp"
#include "common/Noncopyable.hpp"
#include "common/ThreadLocalSingleton.hpp"

namespace tair::common {

struct CPUStatInfo {
    double used_sys_ms = 0.0;
    double used_user_ms = 0.0;
    double used_sys_children_ms = 0.0;
    double used_user_children_ms = 0.0;
    double usage_sys_percent = 0.0;
    double usage_user_percent = 0.0;
    double usage_percent = 0.0;
};

class ProcessCPUStatistics : private Noncopyable {
public:
    ProcessCPUStatistics() = default;
    ~ProcessCPUStatistics() = default;

    void calcProcessUsage();
    CPUStatInfo getLastStatistics();

private:
    void updateLastStatistics(const CPUStatInfo &stat);

private:
    int64_t last_sample_time_ = ClockTime::intervalMs();

    Mutex mutex_;
    CPUStatInfo cpu_stat_info_ GUARDED_BY(mutex_);
};

// -------------------------------------------------------------------------------------------------

struct ThreadCPUStatInfo {
    double used_sys_ms_interval = 0.0;
    double used_user_ms_interval = 0.0;
    double usage_sys_percent = 0.0;
    double usage_user_percent = 0.0;
    double usage_percent = 0.0;
    int64_t interval_ms = 0;
};

class ThreadCPUStatistics : private Noncopyable {
public:
    ThreadCPUStatistics() = default;
    ~ThreadCPUStatistics() = default;

    void setThreadName(const std::string &name) { thread_name_ = name; }
    std::string getThreadName() const { return thread_name_; }

    void calcCurrentThreadUsage();
    ThreadCPUStatInfo getLastStatistics();

private:
    void updateLastStatistics(const ThreadCPUStatInfo &stat);

private:
    int64_t last_sample_time_ = ClockTime::intervalMs();
    std::string thread_name_ = "thread-?";
    double prev_used_sys_ms_ = 0.0;
    double prev_used_user_ms_ = 0.0;

    Mutex mutex_;
    ThreadCPUStatInfo thread_cpu_stat_info_ GUARDED_BY(mutex_);
};

class ThreadCPUStatisticsHelper : private Noncopyable {
private:
    using LocalCPUStatistics = ThreadLocalSingleton<ThreadCPUStatistics>;
    using Callback = LocalCPUStatistics::Callback;

public:
    static void setThreadName(const std::string &name) {
        LocalCPUStatistics::instance()->setThreadName(name);
    }

    static void calcCurrentThreadUsage() {
        LocalCPUStatistics::instance()->calcCurrentThreadUsage();
    }

    static void rangeForAllThreadData(const Callback &callback) {
        LocalCPUStatistics::rangeForAllThreadData(callback);
    }
};

} // namespace tair::common
