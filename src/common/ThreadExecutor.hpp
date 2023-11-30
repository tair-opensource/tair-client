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

#include <atomic>
#include <deque>
#include <functional>
#include <future>
#include <string>
#include <thread>
#include <utility>

#include "common/Assert.hpp"
#include "common/ClockTime.hpp"
#include "common/Logger.hpp"
#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"

namespace tair::common {

class ThreadExecutor final : private Noncopyable {
public:
    static const constexpr int DEFAULT_SLEEP_TIME_MS = 1000;
    typedef std::function<void(ThreadExecutor *)> ThreadExecutorCallback;
    typedef std::function<void()> Task;

public:
    explicit ThreadExecutor(size_t capacity, std::string name = "", bool allow_busy_loop = false)
        : capacity_(capacity), name_(std::move(name)), allow_busy_loop_(allow_busy_loop) {}
    ~ThreadExecutor();

    void setInitCallback(const ThreadExecutorCallback &callback) {
        runtimeAssert(stopped_);
        init_callback_ = callback;
    }

    void setAfterTaskCallback(const ThreadExecutorCallback &callback) {
        runtimeAssert(stopped_);
        after_task_callback_ = callback;
    }

    void setCronCallback(const ThreadExecutorCallback &callback, int64_t cron_time_ms) {
        runtimeAssert(stopped_);
        cron_callback_ = callback;
        cron_time_ms_ = cron_time_ms;
    }

    void start();
    void stop();

    bool putTask(const Task &task, bool force = false) {
        constexpr auto BUSY_LOOP_SLEEP_US = 10;
        if (!stopped_) {
            while (capacity_ != 0 && queueSize() > capacity_ && !force) {
                std::this_thread::sleep_for(std::chrono::microseconds(BUSY_LOOP_SLEEP_US));
                if (stopped_) {
                    return false;
                }
            }
            LockGuard lock(mutex_);
            queue_.emplace_back(task);
            if (need_notify_) {
                not_empty_cond_.notifyOne();
                need_notify_ = false;
            }
            return true;
        }
        return false;
    }

    std::future<void> putTaskWithPromise(Task &&task) {
        auto promise = std::make_shared<std::promise<void>>();
        std::future<void> future = promise->get_future();
        auto promise_task = [task = std::move(task), promise]() {
            task();
            promise->set_value();
        };
        bool success = putTask(std::move(promise_task), true);
        if (!success) {
            LOG_WARN("put task failed, maybe executor is stopped");
            promise->set_value();
        }
        return future;
    }

    const std::string &getName() const {
        return name_;
    }

    size_t queueSize() const {
        LockGuard lock(mutex_);
        return queue_.size();
    }

private:
    void cronCallbackCheck();
    void threadFunc();

private:
    std::atomic<bool> stopped_ = true;

    mutable Mutex mutex_;
    Condition not_empty_cond_;
    std::deque<Task> queue_ GUARDED_BY(mutex_);
    const size_t capacity_ = 0;
    const std::string name_;
    const bool allow_busy_loop_ = false;

    bool need_notify_ GUARDED_BY(mutex_) = true;
    std::unique_ptr<std::thread> thread_;

    int64_t cron_time_ms_ = DEFAULT_SLEEP_TIME_MS;
    int64_t prev_cron_time_ms_ = ClockTime::intervalMs();

    ThreadExecutorCallback init_callback_;
    ThreadExecutorCallback after_task_callback_;
    ThreadExecutorCallback cron_callback_;
};

} // namespace tair::common
