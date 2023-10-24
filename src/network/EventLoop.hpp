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

#include <any>
#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>

#include "common/Assert.hpp"
#include "common/Mutex.hpp"
#include "network/EventWatcher.hpp"

namespace tair::network {

using common::Noncopyable;
using common::Mutex;
using common::LockGuard;

class EventLoop final : private Noncopyable {
public:
    EventLoop(const std::string &name = "");
    ~EventLoop();

    void run();
    void stop();

    const std::string &getName() const {
        return name_;
    }

    std::string getBackendName();

    void wakeUpLoop() {
        if (!wake_up_notified_) {
            if (wake_up_watcher_) {
                wake_up_notified_ = true;
                wake_up_watcher_->notify();
            }
        }
    }

    template <typename EXPECTED, typename TASK>
    void runInLoopMaybeRedir(EXPECTED &&expected_cb, TASK &&task_cb) {
        if (isInLoopThread()) {
            task_cb(this);
        } else {
            queueInLoopMaybeRedir(std::forward<EXPECTED>(expected_cb), std::forward<TASK>(task_cb));
        }
    }

    template <typename EXPECTED, typename TASK>
    void queueInLoopMaybeRedir(EXPECTED &&expected_cb, TASK &&task_cb) {
        {
            LockGuard lock(mutex_);
            auto callback_pair = std::make_pair(std::forward<EXPECTED>(expected_cb), std::forward<TASK>(task_cb));
            pending_functors_.emplace_back(callback_pair);
        }
        if (!pending_notified_) {
            if (pending_watcher_) {
                pending_notified_ = true;
                pending_watcher_->notify();
            } else {
                runtimeAssert(!isRunning());
            }
        }
    }

    template <typename TASK>
    void runInLoop(TASK &&task_cb) {
        runInLoopMaybeRedir(nullptr, std::forward<TASK>(task_cb));
    }

    template <typename TASK>
    void queueInLoop(TASK &&task_cb) {
        queueInLoopMaybeRedir(nullptr, std::forward<TASK>(task_cb));
    }

    // must call it in loop thread
    template <typename TASK>
    static inline void queueInSelfLoop(TASK &&task_cb) {
        runtimeAssert(local_self_loop != nullptr);
        local_self_loop->queueInLoop(std::forward<TASK>(task_cb));
    }

    static inline EventLoop *getSelfLoop() {
        return local_self_loop;
    }

    size_t getLoopEventsCount() const;
    size_t pendingQueueSize() const;
    bool hasPendingTask() const;

    inline struct event_base *getEventBase() const {
        return evbase_;
    }

    inline bool isInLoopThread() const {
        return tid_ == std::this_thread::get_id();
    }

    inline bool isRunning() const {
        return !stopped_;
    }

    size_t getTimerCount() const {
        return timer_count_;
    }

    TimerId runAfterTimer(Duration delay, const TimerHandler &callback);
    TimerId runEveryTimer(Duration interval, const TimerHandler &callback);
    void cancelTimer(TimerId id);

    void setContext(const std::any &context) {
        context_ = context;
    }

    const std::any &getContext() const {
        return context_;
    }

    void setBeforeSleepCallBack(const EventLoopBeforeSleepCallBack &callback) {
        before_sleep_call_back_ = callback;
    }

    void setAfterSleepCallBack(const EventLoopAfterSleepCallBack &callback) {
        after_sleep_call_back_ = callback;
    }

private:
    static thread_local EventLoop *local_self_loop;

private:
    void doPendingFunctors();
    void stopInLoop();

    TimerId runTimer(Duration duration, const TimerHandler &callback, bool periodic);
    void runTimerInLoop(Duration duration, const TimerHandler &callback, bool periodic, TimerId id);
    void cancelTimerInLoop(TimerId id);

    void onTimerTriggered(TimerId id, bool periodic, const TimerHandler &callback);

private:
    std::string name_;
    std::atomic<bool> stopped_;
    struct event_base *evbase_;

    std::unique_ptr<PipeEventWatcher> wake_up_watcher_;
    std::atomic<bool> wake_up_notified_;

    EventLoopBeforeSleepCallBack before_sleep_call_back_;
    EventLoopAfterSleepCallBack after_sleep_call_back_;

    std::unique_ptr<PipeEventWatcher> pending_watcher_;
    std::atomic<bool> pending_notified_;
    mutable Mutex mutex_;
    bool do_pending_ GUARDED_BY(mutex_);
    std::deque<std::pair<ExpectedLoopCallback, LoopTaskCallback>> pending_functors_ GUARDED_BY(mutex_);

    std::atomic<TimerId> next_timer_id_;
    size_t timer_count_; // for debug get
    std::unordered_map<TimerId, std::unique_ptr<Timer>> timer_map_;

    std::thread::id tid_;
    std::any context_;
};

} // namespace tair::network
