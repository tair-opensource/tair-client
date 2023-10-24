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
#include "common/ThreadPool.hpp"

#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"

namespace tair::common {

ThreadPool::ThreadPool(int thread_nums, std::string name)
    : thread_nums_(thread_nums), name_(name) {
    runtimeAssert(thread_nums > 0);
}

ThreadPool::~ThreadPool() {
    if (!stopped_) {
        stop();
    }
}

void ThreadPool::start() {
    runtimeAssert(threads_.empty());
    if (stopped_) {
        stopped_ = false;
        threads_.reserve(thread_nums_);
        for (int i = 0; i < thread_nums_; ++i) {
            threads_.emplace_back(std::make_unique<std::thread>(&ThreadPool::threadFunc, this));
        }
    }
}

void ThreadPool::stop() {
    if (!stopped_) {
        {
            LockGuard lock(mutex_);
            stopped_ = true;
            not_empty_cond_.notifyAll();
        }
        for (auto &thread : threads_) {
            thread->join();
        }
        threads_.clear();
        {
            LockGuard lock(mutex_);
            while (!queue_.empty()) {
                Task task = queue_.front();
                queue_.pop_front();
                task();
            }
        }
    }
}

ThreadPool::Task ThreadPool::take() {
    UniqueLock lock(mutex_);
    while (queue_.empty() && !stopped_) {
        not_empty_cond_.wait(lock);
    }
    Task task;
    if (!queue_.empty()) {
        task = queue_.front();
        queue_.pop_front();
        if (max_queue_size_ > 0) {
            not_full_cond_.notifyOne();
        }
    }
    return task;
}

void ThreadPool::threadFunc() {
    std::string thread_name = std::string("thp-") + name_;
    SystemUtil::setThreadName(thread_name);
    while (!stopped_) {
        Task task(take());
        if (task) {
            task();
        }
    }
    LOG_DEBUG("ThreadPool thread exit, name: {}", thread_name);
}

} // namespace tair::common
