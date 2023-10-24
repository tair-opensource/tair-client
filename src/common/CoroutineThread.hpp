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
#include <list>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>

#include "common/Assert.hpp"
#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"

typedef struct aco_s aco_t;
typedef struct aco_share_stack_s aco_share_stack_t;

namespace tair::common {

class CoroutineThread final : private Noncopyable {
public:
    typedef std::function<void()> Task;

public:
    CoroutineThread(bool use_shared_stack = false, const std::string &name = std::string());
    ~CoroutineThread();

    void start();
    void stop();

    template <typename T>
    bool putTask(T &&task) EXCLUDES(mutex_) {
        if (!stopped_) {
            UniqueLock lock(mutex_);
            if (max_queue_size_ > 0 && queue_.size() >= max_queue_size_) {
                return false;
            }
            queue_.emplace_back(std::forward<T>(task));
            not_empty_cond_.notifyOne();
            return true;
        }
        return false;
    }

    void setInitCallback(const Task &callback) {
        init_callback_ = callback;
    }

    void setIdleCallback(const Task &callback) {
        idle_callback_ = callback;
    }

    void setSleepTimeMs(int time_ms) {
        sleep_time_ms_ = time_ms;
    }

    void setMaxQueueSize(size_t maxSize) {
        max_queue_size_ = maxSize;
    }

    void setMaxCoroutineSize(size_t maxSize) {
        max_coroutine_size_ = maxSize;
    }

    size_t queueSize() const EXCLUDES(mutex_) {
        LockGuard lock(mutex_);
        return queue_.size();
    }

    size_t coroutineSize() const {
        return coroutines_size_;
    }

    int64_t getMinCoroutinesStartTime() const EXCLUDES(co_time_mutex_);

    static bool isCallInCoroutineThread() { return this_coroutine_thread_ != nullptr; }
    static void yield();
    static int64_t getSelfMinCoroutinesStartTime();

private:
    void initMainCoroutine();
    void createCoroutine(Task *task);
    void destroyCoroutine(aco_t *co);
    void resumeCoroutine();
    void registerCoroutine(aco_t *co);
    void unregisterCoroutine(aco_t *co);
    Task take();
    void threadFunc();

private:
    std::atomic<bool> stopped_ = true;
    bool use_shared_stack_ = false;
    std::string name_;
    size_t max_queue_size_ = 0;
    size_t max_coroutine_size_ = 0;
    std::atomic<size_t> coroutines_size_ = 0;

    mutable Mutex mutex_;
    Condition not_empty_cond_;
    std::deque<Task> queue_ GUARDED_BY(mutex_);
    Task init_callback_;
    Task idle_callback_;
    int sleep_time_ms_ = 10;

    aco_t *main_co_ = nullptr;
    aco_share_stack_t *shared_stack_ = nullptr;
    std::list<aco_t *> coroutines_;
    std::unique_ptr<std::thread> thread_;
    thread_local static CoroutineThread *this_coroutine_thread_;

    mutable Mutex co_time_mutex_;
    std::multiset<int64_t> co_start_times_ GUARDED_BY(co_time_mutex_);
    std::unordered_map<aco_t *, std::set<int64_t>::iterator> co_start_times_map_ GUARDED_BY(co_time_mutex_);
};

} // namespace tair::common
