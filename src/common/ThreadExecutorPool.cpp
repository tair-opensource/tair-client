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
#include "common/ThreadExecutorPool.hpp"

#include "common/Logger.hpp"

namespace tair::common {

ThreadExecutorPool::ThreadExecutorPool(int executor_num, size_t capacity_per_executor, const std::string &name) {
    executor_num_ = executor_num;
    capacity_per_executor_ = capacity_per_executor;
    runtimeAssert(executor_num_ >= 0);
    name_ = "th-" + name;
    executors_.resize(executor_num);
}

ThreadExecutorPool::~ThreadExecutorPool() {
    stop();
}

void ThreadExecutorPool::start() {
    if (stopped_) {
        LOG_DEBUG("ThreadExecutorPool start, executor_num={}, name: {}", executor_num_, name_);
        stopped_ = false;
        executors_.reserve(executor_num_);
        for (int i = 0; i < executor_num_; ++i) {
            std::string executor_name = name_ + "-" + std::to_string(i);
            auto executor = std::make_unique<ThreadExecutor>(capacity_per_executor_, executor_name);
            executor->setInitCallback(init_callback_);
            executor->setAfterTaskCallback(after_task_callback_);
            executor->setCronCallback(cron_callback_, cron_time_ms_);
            executor->start();
            executors_[i] = std::move(executor);
        }
    }
}

void ThreadExecutorPool::stop() {
    if (!stopped_) {
        LOG_DEBUG("ThreadExecutorPool stop, executor_num={}, name: {}", executor_num_, name_);
        for (auto &executor : executors_) {
            executor->stop();
        }
        stopped_ = true;
    }
}

ThreadExecutor *ThreadExecutorPool::getNextExecutor() {
    size_t index = next_.fetch_add(1) % executor_num_;
    return executors_[index].get();
}

ThreadExecutor *ThreadExecutorPool::getNextExecutor(int index) {
    return executors_[index].get();
}

ThreadExecutor *ThreadExecutorPool::getNextExecutorWithHash(uint64_t hash) {
    size_t index = hash % executor_num_;
    return executors_[index].get();
}

size_t ThreadExecutorPool::allExecutorQueueSize() const {
    size_t all_size = 0;
    for (auto &executor : executors_) {
        all_size += executor->queueSize();
    }
    return all_size;
}

} // namespace tair::common
