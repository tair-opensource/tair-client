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
#include "common/CPUStatistics.hpp"

#include <sys/resource.h>

#if defined(__APPLE__)
#include <mach/mach_init.h>
#include <mach/thread_act.h>
#endif

#include "fmt/format.h"

namespace tair::common {

void ProcessCPUStatistics::calcProcessUsage() {
    struct rusage self_ru;
    ::getrusage(RUSAGE_SELF, &self_ru);

    CPUStatInfo stat = getLastStatistics();

    double prev_sys_ms = stat.used_sys_ms;
    double prev_user_ms = stat.used_user_ms;

    stat.used_sys_ms = (self_ru.ru_stime.tv_sec * 1000.0) + (self_ru.ru_stime.tv_usec / 1000.0);
    stat.used_user_ms = (self_ru.ru_utime.tv_sec * 1000.0) + (self_ru.ru_utime.tv_usec / 1000.0);

    int64_t curr_time_ms = ClockTime::intervalMs();
    int64_t interval_ms = curr_time_ms - last_sample_time_;
    last_sample_time_ = curr_time_ms;

    stat.usage_sys_percent = (stat.used_sys_ms - prev_sys_ms) * 100.0 / interval_ms;
    stat.usage_user_percent = (stat.used_user_ms - prev_user_ms) * 100.0 / interval_ms;
    stat.usage_percent = stat.usage_sys_percent + stat.usage_user_percent;

    struct rusage children_ru;
    ::getrusage(RUSAGE_CHILDREN, &children_ru);
    stat.used_sys_children_ms = (children_ru.ru_stime.tv_sec * 1000.0) + (children_ru.ru_stime.tv_usec / 1000.0);
    stat.used_user_children_ms = (children_ru.ru_utime.tv_sec * 1000.0) + (children_ru.ru_utime.tv_usec / 1000.0);

    updateLastStatistics(stat);
}

CPUStatInfo ProcessCPUStatistics::getLastStatistics() {
    LockGuard lock(mutex_);
    return cpu_stat_info_;
}

void ProcessCPUStatistics::updateLastStatistics(const CPUStatInfo &stat) {
    LockGuard lock(mutex_);
    cpu_stat_info_ = stat;
}

// -------------------------------------------------------------------------------------------------

void ThreadCPUStatistics::calcCurrentThreadUsage() {
    struct rusage self_ru;

#if defined(__linux__)
    ::getrusage(RUSAGE_THREAD, &self_ru);
#elif defined(__APPLE__)
    mach_port_t thread = ::pthread_mach_thread_np(pthread_self());
    mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
    thread_basic_info_data_t info;
    kern_return_t kr = ::thread_info(thread, THREAD_BASIC_INFO, (thread_info_t)&info, &count);
    if (kr == KERN_SUCCESS && (info.flags & TH_FLAGS_IDLE) == 0) {
        self_ru.ru_utime.tv_sec = info.user_time.seconds;
        self_ru.ru_utime.tv_usec = info.user_time.microseconds;
        self_ru.ru_stime.tv_sec = info.system_time.seconds;
        self_ru.ru_stime.tv_usec = info.system_time.microseconds;
    }
#else
    memset(&self_ru, 0, sizeof(self_ru));
#endif
    ThreadCPUStatInfo stat = getLastStatistics();

    double used_sys_ms = (self_ru.ru_stime.tv_sec * 1000.0) + (self_ru.ru_stime.tv_usec / 1000.0);
    double used_user_ms = (self_ru.ru_utime.tv_sec * 1000.0) + (self_ru.ru_utime.tv_usec / 1000.0);

    int64_t curr_time_ms = ClockTime::intervalMs();
    int64_t interval_ms = curr_time_ms - last_sample_time_;
    last_sample_time_ = curr_time_ms;

    stat.used_sys_ms_interval = used_sys_ms - prev_used_sys_ms_;
    stat.usage_sys_percent = stat.used_sys_ms_interval * 100.0 / interval_ms;
    stat.used_user_ms_interval = used_user_ms - prev_used_user_ms_;
    stat.usage_user_percent = stat.used_user_ms_interval * 100.0 / interval_ms;
    stat.usage_percent = stat.usage_sys_percent + stat.usage_user_percent;
    if (stat.usage_percent > 100) {
        stat.usage_percent = 100;
    }
    stat.interval_ms = interval_ms;

    prev_used_sys_ms_ = used_sys_ms;
    prev_used_user_ms_ = used_user_ms;

    updateLastStatistics(stat);
}

ThreadCPUStatInfo ThreadCPUStatistics::getLastStatistics() {
    LockGuard lock(mutex_);
    return thread_cpu_stat_info_;
}

void ThreadCPUStatistics::updateLastStatistics(const ThreadCPUStatInfo &stat) {
    LockGuard lock(mutex_);
    thread_cpu_stat_info_ = stat;
}

} // namespace tair::common
