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

#include <mutex>
#include "common/StringUtil.hpp"

#include "fmt/format.h"

namespace tair::common {

class DateTime {
public:

    static inline std::string getDateTime(const char *time_fmt, time_t time) {
        static std::mutex mutex;
        static std::once_flag once_flag;
        std::call_once(once_flag, []() {
            pthread_atfork([]() { mutex.lock(); }, []() { mutex.unlock(); }, []() { mutex.unlock(); });
        });
        char time_buf[128] = {0};
        struct tm now_tm {};
        {
            std::unique_lock<std::mutex> lock(mutex);
        ::localtime_r(&time, &now_tm);
        }
        ::strftime(time_buf, sizeof(time_buf), time_fmt, &now_tm);
        return std::string(time_buf);
    }

    static inline std::string getDateTimeMs(int64_t time_ms) {
        time_t time_s = (time_t)(time_ms / 1000);
        int64_t ms = time_ms % 1000;
        return fmt::format("{}.{:0>3}", getDateTime("%Y-%m-%d %H:%M:%S", time_s), ms);
    }

    static inline int64_t parseDateTimeMs(const std::string &strtime) {
        auto result = StringUtil::split(strtime, '.');
        if (result.size() != 2) {
            return 0;
        }
        struct tm time_tm {};
        if (!::strptime(result[0].c_str(), "%Y-%m-%d %H:%M:%S", &time_tm)) {
            return 0;
        }
        int64_t time_s = ::mktime(&time_tm);
        return time_s * 1000 + ::atoi(result[1].c_str());
    }
};

} // namespace tair::common
