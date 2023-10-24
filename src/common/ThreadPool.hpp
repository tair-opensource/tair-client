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
#include <string>
#include <thread>
#include <vector>

#include "common/Assert.hpp"
#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"

namespace tair::common {

class ThreadPool final : private Noncopyable {
public:
    typedef std::function<void()> Task;

public:
    explicit ThreadPool(int thread_nums, std::string name = std::string());
    ~ThreadPool();

    void start();
    void stop();

    const bool isStopped() const {
        return stopped_;
    }

    template <typename T>
    void putTask(T &&task) {
        if (!stopped_) {
            UniqueLock lock(mutex_);
            while (max_queue_size_ > 0 && queue_.size() >= max_queue_size_) {
                not_full_cond_.wait(lock);
            }
            runtimeAssert(max_queue_size_ == 0 || queue_.size() < max_queue_size_);
            queue_.emplace_back(std::forward<T>(task));
            not_empty_cond_.notifyOne();
        } else {
            task();
        }
    }

    void setMaxQueueSize(size_t maxSize) {
        max_queue_size_ = maxSize;
    }

    size_t queueSize() const {
        LockGuard lock(mutex_);
        return queue_.size();
    }
    const std::string &getName() const {
        return name_;
    }

private:
    Task take();
    void threadFunc();

private:
    std::atomic<bool> stopped_ = true;
    int thread_nums_;
    std::string name_;
    size_t max_queue_size_ = 0;

    mutable Mutex mutex_;
    Condition not_full_cond_;
    Condition not_empty_cond_;
    std::deque<Task> queue_ GUARDED_BY(mutex_);

    std::vector<std::unique_ptr<std::thread>> threads_;
};

} // namespace tair::common
