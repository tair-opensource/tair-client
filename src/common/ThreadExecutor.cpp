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
#include "common/ThreadExecutor.hpp"

#if defined(__x86_64__)
#include <immintrin.h>
static inline void _pause() {
    _mm_pause();
}
#elif defined(__aarch64__)
#include <arm_acle.h>
static inline void _pause() {
    __asm__ __volatile__("yield;"
                         :
                         :
                         : "memory");
}
#else
#error "Unsupported architecture"
#endif
#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"

namespace tair::common {

ThreadExecutor::~ThreadExecutor() {
    if (!stopped_) {
        stop();
    }
}

void ThreadExecutor::start() {
    if (stopped_) {
        stopped_ = false;
        thread_ = std::make_unique<std::thread>(&ThreadExecutor::threadFunc, this);
    }
}

void ThreadExecutor::stop() {
    if (!stopped_) {
        stopped_ = true;
        not_empty_cond_.notifyOne();
        thread_->join();
    }
}

void ThreadExecutor::cronCallbackCheck() {
    if (!cron_callback_) {
        return;
    }
    int64_t now_ms = ClockTime::intervalMs();
    if (now_ms - prev_cron_time_ms_ > cron_time_ms_) {
        prev_cron_time_ms_ = now_ms;
        cron_callback_(this);
    }
}

void ThreadExecutor::threadFunc() {
    std::string thread_name = name_;
    SystemUtil::setThreadName(thread_name);
    if (init_callback_) {
        init_callback_(this);
    }

    std::deque<Task> tmp_queue;
    bool skip_next_busy_loop = true;
    while (!stopped_) {
        constexpr size_t MAX_PAUSE_COUNT = 1000;
        size_t busy_pause_count = 0;
        while (allow_busy_loop_ && !skip_next_busy_loop && !stopped_) {
            if (queueSize() != 0 || ++busy_pause_count > MAX_PAUSE_COUNT) {
                break;
            }
            _pause();
        }
        {
            UniqueLock lock(mutex_);
            if (queue_.empty()) {
                need_notify_ = true;
                not_empty_cond_.waitFor(lock, std::chrono::milliseconds(cron_time_ms_));
                need_notify_ = false;
            }
            if (!queue_.empty()) {
                queue_.swap(tmp_queue);
            }
        }
        cronCallbackCheck();
        for (const auto &task : tmp_queue) {
            task();
            cronCallbackCheck();
        }
        if (!tmp_queue.empty()) {
            if (after_task_callback_) {
            after_task_callback_(this);
            cronCallbackCheck();
            }
            tmp_queue.clear();
            skip_next_busy_loop = false;
        } else {
            skip_next_busy_loop = true;
        }
    }
    LOG_DEBUG("ThreadExecutor exit, name: {}", thread_name);
}

} // namespace tair::common
