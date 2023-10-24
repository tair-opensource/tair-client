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
#include "network/EventLoop.hpp"

#include <sstream>

#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"
#include "network/Sockets.hpp"
#include "network/Timer.hpp"

namespace tair::network {

using common::SystemUtil;

thread_local EventLoop *EventLoop::local_self_loop = nullptr;

EventLoop::EventLoop(const std::string &name)
    : stopped_(true), wake_up_notified_(false), pending_notified_(false), do_pending_(false), next_timer_id_(1), timer_count_(0) {
    if (!name.empty()) {
        name_ = name;
    } else {
        std::stringstream ss;
        ss << "EventLoop-" << this;
        name_ = ss.str();
    }
    tid_ = std::this_thread::get_id();
    LOG_TRACE("loop init in thread: {}", tid_);
    struct event_config *cfg = ::event_config_new();
    // EVENT_BASE_FLAG_NOLOCK: one loop per thread.
    // EVENT_BASE_FLAG_PRECISE_TIMER: use CLOCK_MONOTONIC, CLOCK_MONOTONIC_COARSE causes various issues with typical CONFIG_HZ=250, notably very inacurate.
    // EVENT_BASE_FLAG_EPOLL_DISALLOW_TIMERFD: epoll backend will use timerfd for more accurate timers(if EVENT_BASE_FLAG_PRECISE_TIMER enabled), this will allows to disable this.
    ::event_config_set_flag(cfg, EVENT_BASE_FLAG_NOLOCK | EVENT_BASE_FLAG_PRECISE_TIMER | EVENT_BASE_FLAG_EPOLL_DISALLOW_TIMERFD);
    evbase_ = ::event_base_new_with_config(cfg);
    ::event_config_free(cfg);
    pending_watcher_ = std::make_unique<PipeEventWatcher>(this, [this]() {
        doPendingFunctors();
    });
    if (!pending_watcher_->init()) {
        LOG_CRITICAL("PipeEventWatcher init failed");
    }
    wake_up_watcher_ = std::make_unique<PipeEventWatcher>(this, [this]() { wake_up_notified_ = false; });
    if (!wake_up_watcher_->init()) {
        LOG_CRITICAL("PipeEventWatcher init failed");
    }
    local_self_loop = this;
}

EventLoop::~EventLoop() {
    {
        LockGuard lock(mutex_);
        if (!pending_functors_.empty()) {
            LOG_WARN("pending_functors_ size is {}, some callback give up", pending_functors_.size());
            pending_functors_.clear();
        }
    }
    pending_watcher_.reset();
    wake_up_watcher_.reset();
    timer_map_.clear();
    ::event_base_free(evbase_);
}

std::string EventLoop::getBackendName() {
    return ::event_base_get_method(evbase_);
}

void EventLoop::run() {
    LOG_DEBUG("loop name: {}, tid: {}, run in thread: {}", name_, tid_, std::this_thread::get_id());
    runtimeAssert(local_self_loop == this);
    runtimeAssert(isInLoopThread() && !isRunning());
    if (!pending_watcher_->start()) {
        LOG_CRITICAL("PipeEventWatcher init failed");
    }
    if (!wake_up_watcher_->start()) {
        LOG_CRITICAL("PipeEventWatcher init failed");
    }
    stopped_ = false;
    if (before_sleep_call_back_) {
        ::evwatch_prepare_new(
            evbase_, [](struct evwatch *, const struct evwatch_prepare_cb_info *, void *arg) {
                EventLoop *loop = (EventLoop *)arg;
                loop->before_sleep_call_back_(loop);
            },
            this);
    }
    if (after_sleep_call_back_) {
        ::evwatch_check_new(
            evbase_, [](struct evwatch *, const struct evwatch_check_cb_info *, void *arg) {
                EventLoop *loop = (EventLoop *)arg;
                loop->after_sleep_call_back_(loop);
            },
            this);
    }
    int rc = ::event_base_dispatch(evbase_);
    if (rc == 1) {
        LOG_ERROR("event_base_dispatch error: no event registered");
    } else if (rc == -1) {
        LOG_ERROR("event_base_dispatch errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
    }
    LOG_DEBUG("EventLoop stopped, tid: {}", std::this_thread::get_id());
}

void EventLoop::stop() {
    if (!stopped_) {
        LOG_DEBUG("loop will stop, name: {}, tid: {}, stop in thread: {}", name_, tid_, std::this_thread::get_id());
        stopped_ = true;
        if (isInLoopThread()) {
            stopInLoop();
        } else {
            queueInLoop([this](EventLoop *) {
                stopInLoop();
            });
        }
    }
}

size_t EventLoop::getLoopEventsCount() const {
    return ::event_base_get_num_events(evbase_, EVENT_BASE_COUNT_ADDED);
}

size_t EventLoop::pendingQueueSize() const {
    LockGuard lock(mutex_);
    return pending_functors_.size();
}

bool EventLoop::hasPendingTask() const {
    LockGuard lock(mutex_);
    return !pending_functors_.empty() || do_pending_;
}

void EventLoop::doPendingFunctors() {
    std::deque<std::pair<ExpectedLoopCallback, LoopTaskCallback>> functors, next_loop_functors;
    {
        LockGuard lock(mutex_);
        functors.swap(pending_functors_);
        pending_notified_ = false;
        do_pending_ = true;
    }
    for (auto &[expected_cb, task_cb] : functors) {
        if (expected_cb) {
            auto expected_loop = expected_cb();
            // expected_cb return null, recheck in next loop
            if (!expected_cb) {
                next_loop_functors.emplace_back(std::make_pair(expected_cb, task_cb));
                LOG_TRACE("expected loop is NULL, recheck it in next loop");
                continue;
            }
            // expected_loop not me, redir it
            else if (expected_loop != this) {
                LOG_TRACE("expected loop is not me [{} vs {}], redir it", (void *)expected_loop, (void *)this);
                expected_loop->queueInLoopMaybeRedir(expected_cb, task_cb);
                continue;
            }
        }
        task_cb(this);
    }
    {
        LockGuard lock(mutex_);
        do_pending_ = false;
    }
    for (auto &[expected_cb, task_cb] : next_loop_functors) {
        queueInLoopMaybeRedir(expected_cb, task_cb);
    }
}

void EventLoop::stopInLoop() {
    LOG_DEBUG("loop stop, name: {}, tid: {}, stop in thread: {}", name_, tid_, std::this_thread::get_id());
    runtimeAssert(isInLoopThread());
    ::event_base_loopexit(evbase_, nullptr);
}

TimerId EventLoop::runAfterTimer(Duration delay, const TimerHandler &callback) {
    runtimeAssert(delay.nanoseconds() > 0);
    return runTimer(delay, callback, false);
}

TimerId EventLoop::runEveryTimer(Duration interval, const TimerHandler &callback) {
    return runTimer(interval, callback, true);
}

TimerId EventLoop::runTimer(Duration duration, const TimerHandler &callback, bool periodic) {
    TimerId id = next_timer_id_.fetch_add(1);
    if (isInLoopThread()) {
        runTimerInLoop(duration, callback, periodic, id);
    } else {
        queueInLoop([this, duration, callback, periodic, id](EventLoop *) {
            runTimerInLoop(duration, callback, periodic, id);
        });
    }
    return id;
}

void EventLoop::runTimerInLoop(Duration duration, const TimerHandler &callback, bool periodic, TimerId id) {
    runtimeAssert(isInLoopThread());
    runtimeAssert(timer_map_.find(id) == timer_map_.end());
    auto timer_callback = [this, id, periodic, callback]() {
        onTimerTriggered(id, periodic, callback);
    };
    auto timer = std::make_unique<Timer>(this, timer_callback, duration, periodic, id);
    timer->start();
    timer_map_[id] = std::move(timer);
    timer_count_++;
    runtimeAssert(timer_count_ == timer_map_.size());
    //    LOG_TRACE("Timer start, tid: {}, loop: {}, id: {}, timeout: {} ms, periodic: {}",
    //            tid_, name_, id, duration.milliseconds(), periodic);
}

void EventLoop::cancelTimer(TimerId id) {
    if (isInLoopThread()) {
        cancelTimerInLoop(id);
    } else {
        queueInLoop([this, id](EventLoop *) {
            cancelTimerInLoop(id);
        });
    }
}

void EventLoop::cancelTimerInLoop(TimerId id) {
    runtimeAssert(isInLoopThread());
    auto iter = timer_map_.find(id);
    if (iter != timer_map_.end()) {
        LOG_TRACE("Timer cancel, tid: {}, loop: {}, id: {}, timeout: {} ms, periodic: {}",
                  tid_, name_, id, iter->second->timeout().milliseconds(), iter->second->isPeriodic());
        iter->second->cancel();
        timer_map_.erase(iter);
        timer_count_--;
        runtimeAssert(timer_count_ == timer_map_.size());
    } else {
        LOG_TRACE("Timer cancel, tid: {}, loop: {}, id: {}, but it was canceled or remove by trigger", tid_, name_, id);
    }
}

void EventLoop::onTimerTriggered(TimerId id, bool periodic, const TimerHandler &callback) {
    runtimeAssert(isInLoopThread());
    callback(this);
    auto iter = timer_map_.find(id);
    if (iter == timer_map_.end()) {
        // this timer removed in callback(), return now
        return;
    }
    //    LOG_TRACE("Timer triggered, tid: {}, loop: {}, id: {}, timeout: {} ms, periodic:{}",
    //            tid_, this, id, iter->second->timeout().milliseconds(), iter->second->isPeriodic());
    if (!periodic) {
        iter->second->cancel();
        timer_map_.erase(iter);
        timer_count_--;
        runtimeAssert(timer_count_ == timer_map_.size());
    }
}

} // namespace tair::network
