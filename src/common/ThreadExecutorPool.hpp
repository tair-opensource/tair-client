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
#include <string>
#include <vector>

#include "common/Assert.hpp"
#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"
#include "common/ThreadExecutor.hpp"

namespace tair::common {

using ThreadExecutorCallback = ThreadExecutor::ThreadExecutorCallback;
using Task = ThreadExecutor::Task;

class ThreadExecutorPool final : private Noncopyable {
public:
    ThreadExecutorPool(int executor_num, size_t capacity_per_executor, const std::string &name = "");
    ~ThreadExecutorPool();

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

    ThreadExecutor *getNextExecutor();
    ThreadExecutor *getNextExecutor(int index);
    ThreadExecutor *getNextExecutorWithHash(uint64_t hash);

    size_t allExecutorQueueSize() const;

    bool isRunning() const {
        return !stopped_;
    }

    int executorNum() const {
        return executor_num_;
    }

    const std::string &getName() const {
        return name_;
    }

private:
    std::atomic<bool> stopped_ = true;
    int executor_num_;
    size_t capacity_per_executor_;
    std::string name_;
    ThreadExecutorCallback init_callback_;
    ThreadExecutorCallback after_task_callback_;
    ThreadExecutorCallback cron_callback_;
    int64_t cron_time_ms_ = ThreadExecutor::DEFAULT_SLEEP_TIME_MS;
    std::atomic<uint64_t> next_ = 0;
    std::vector<std::unique_ptr<ThreadExecutor>> executors_;
};

} // namespace tair::common
