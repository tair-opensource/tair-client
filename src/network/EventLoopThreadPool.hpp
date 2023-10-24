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
#include <memory>
#include <vector>

#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"
#include "network/EventLoopThread.hpp"

namespace tair::network {

using common::Noncopyable;
using common::ReadWriteLock;
using common::ReadLockGuard;
using common::WriteLockGuard;

using EventLoopThreadPtr = std::shared_ptr<EventLoopThread>;

class EventLoopThreadPool : private Noncopyable {
public:
    EventLoopThreadPool(EventLoop *base_loop, size_t thread_num, std::string name = "");
    ~EventLoopThreadPool();

    void setLoopInitCallback(const EventLoopInitCallback &callback) {
        init_callback_ = callback;
    }

    void setLoopDestroyCallback(const EventLoopDestroyCallback &callback) {
        destroy_callback_ = callback;
    }

    void setBeforeSleepCallBack(const EventLoopBeforeSleepCallBack &callback) {
        before_sleep_call_back_ = callback;
    }

    void setAfterSleepCallBack(const EventLoopAfterSleepCallBack &callback) {
        after_sleep_call_back_ = callback;
    }

    void setAfterResizeThreadExitCheckCallBack(const EventLoopThreadExitCheckCallBack &callback) {
        thread_exit_check_callback_ = callback;
    }

    void start();
    void stop();
    void join();

    bool isRunning() const {
        return !stopped_;
    }

    bool resizeThreadPoolSize(size_t new_thread_num) EXCLUDES(rw_lock_);

    size_t ioThreadNum() const EXCLUDES(rw_lock_) {
        ReadLockGuard rlock(rw_lock_);
        return thread_num_;
    }

    size_t availableIOThreadNum() const EXCLUDES(rw_lock_) {
        ReadLockGuard rlock(rw_lock_);
        return available_thread_num_;
    }

    void runInNextlLoop(const LoopTaskCallback &callback) EXCLUDES(rw_lock_);
    void runInLoopByHash(size_t hash, const LoopTaskCallback &callback) EXCLUDES(rw_lock_);

    size_t runInAllLoop(const LoopTaskCallback &callback) EXCLUDES(rw_lock_);
    size_t queueInAllLoop(const LoopTaskCallback &callback) EXCLUDES(rw_lock_);
    size_t runInChoosedLoop(const ChooseLoopCallback &choose_callback, const LoopTaskCallback &callback) EXCLUDES(rw_lock_);
    size_t runInRandomLoop(const LoopTaskCallback &callback) EXCLUDES(rw_lock_);

    void runWithAllLoop(const AllLoopTaskCallback &callback) EXCLUDES(rw_lock_);

    constexpr static const size_t MAX_THREAD_POOL_SIZE = 128;

private:
    void createLoopThread(int idx) REQUIRES(rw_lock_);
    void checkAndStopLoopThread() REQUIRES(rw_lock_);

private:
    std::atomic<bool> stopped_ = true;
    std::string name_;

    EventLoopInitCallback init_callback_;
    EventLoopDestroyCallback destroy_callback_;
    EventLoopBeforeSleepCallBack before_sleep_call_back_;
    EventLoopAfterSleepCallBack after_sleep_call_back_;
    EventLoopThreadExitCheckCallBack thread_exit_check_callback_;

    EventLoop *base_loop_;
    std::atomic<size_t> next_loop_index_;

    mutable ReadWriteLock rw_lock_;
    size_t thread_num_ GUARDED_BY(rw_lock_);
    size_t available_thread_num_ GUARDED_BY(rw_lock_);
    std::vector<EventLoopThreadPtr> threads_ GUARDED_BY(rw_lock_);
    std::vector<EventLoop *> loops_ GUARDED_BY(rw_lock_);
};

} // namespace tair::network
