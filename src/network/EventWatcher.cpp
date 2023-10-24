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
#include "network/EventWatcher.hpp"

#include "common/Compiler.hpp"
#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"
#include "network/EventLoop.hpp"
#include "network/Sockets.hpp"

namespace tair::network {

using common::SystemUtil;

EventWatcher::EventWatcher(EventLoop *loop, const Callback &callback)
    : loop_(loop), attached_(false), canceled_(false), callback_(callback) {
    event_ = new event;
    ::memset(event_, 0, sizeof(struct event));
}

EventWatcher::~EventWatcher() {
    cancel();
}

bool EventWatcher::init() {
    runtimeAssert(loop_->isInLoopThread());
    if (!doInit()) {
        closeEvent();
        return false;
    }
    return (::event_base_set(loop_->getEventBase(), event_) == 0);
}

void EventWatcher::cancel() {
    runtimeAssert(loop_->isInLoopThread());
    if (!canceled_) {
        closeEvent();
        freeEvent();
        canceled_ = true;
    }
}

void EventWatcher::closeEvent() {
    doClose();
}

void EventWatcher::freeEvent() {
    if (event_) {
        if (attached_) {
            ::event_del(event_);
            attached_ = false;
        }
        delete event_;
        event_ = nullptr;
    }
}

bool EventWatcher::attachToEventLoop(Duration timeout) {
    runtimeAssert(loop_->isInLoopThread());
    runtimeAssert(::event_initialized(event_));
    runtimeAssert(!attached_);
    struct timeval tv;
    struct timeval *timeoutval = nullptr;
    if (timeout.nanoseconds() > 0) {
        timeout.to(&tv);
        timeoutval = &tv;
    }
    if (::event_add(event_, timeoutval) != 0) {
        LOG_ERROR("event_add failed. fd={} event_={}", event_->ev_fd, (void *)event_);
        return false;
    }
    attached_ = true;
    return true;
}

// ---------------------------------------------------------------------------------

PipeEventWatcher::PipeEventWatcher(EventLoop *loop, const Callback &callback)
    : EventWatcher(loop, callback) {
}

PipeEventWatcher::~PipeEventWatcher() {
    closeEvent();
}

bool PipeEventWatcher::start() {
    return EventWatcher::attachToEventLoop(Duration(0));
}

void PipeEventWatcher::notify() {
    char event = 'a';
    sockets::writeToSocket(pipe_[0], &event, sizeof(event));
}

bool PipeEventWatcher::doInit() {
    runtimeAssert(pipe_[0] == -1 && pipe_[1] == -1);
    if (sockets::createSocketPair(AF_UNIX, SOCK_STREAM, 0, pipe_) < 0 || sockets::setNonBlocking(pipe_[0]) < 0 || sockets::setNonBlocking(pipe_[1]) < 0) {
        LOG_ERROR("create socketpair ERROR errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
        closeEvent();
        return false;
    }
    ::event_set(event_, pipe_[1], EV_READ | EV_PERSIST, &PipeEventWatcher::handlerFn, this);

    return true;
}

void PipeEventWatcher::doClose() {
    if (pipe_[0] > 0) {
        sockets::closeSocket(pipe_[0]);
        sockets::closeSocket(pipe_[1]);
        pipe_[0] = -1;
        pipe_[1] = -1;
    }
}

void PipeEventWatcher::handlerFn(int fd UNUSED, short which UNUSED, void *v) {
    PipeEventWatcher *watcher = (PipeEventWatcher *)v;
    char buf[128];
    int n = sockets::readFromSocket(watcher->pipe_[1], buf, sizeof(buf));
    if (n > 0) {
        watcher->callback_();
    }
}

// ---------------------------------------------------------------------------------

TimerEventWatcher::TimerEventWatcher(EventLoop *loop, const Callback &callback, Duration timeout, bool periodic)
    : EventWatcher(loop, callback), timeout_(timeout), periodic_(periodic) {
}

bool TimerEventWatcher::start() {
    return EventWatcher::attachToEventLoop(timeout_);
}

bool TimerEventWatcher::doInit() {
    if (periodic_) {
        ::event_set(event_, -1, EV_PERSIST, TimerEventWatcher::handlerFn, this);
    } else {
        ::event_set(event_, -1, 0, TimerEventWatcher::handlerFn, this);
    }
    return true;
}

void TimerEventWatcher::handlerFn(int fd UNUSED, short which UNUSED, void *v) {
    TimerEventWatcher *watcher = (TimerEventWatcher *)v;
    watcher->callback_();
}

// ---------------------------------------------------------------------------------

SignalEventWatcher::SignalEventWatcher(int signo, EventLoop *loop, const Callback &callback)
    : EventWatcher(loop, callback), signo_(signo) {
}

bool SignalEventWatcher::start() {
    return EventWatcher::attachToEventLoop(Duration(0));
}

bool SignalEventWatcher::doInit() {
    runtimeAssert(signo_ != 0);
    ::signal_set(event_, signo_, SignalEventWatcher::handlerFn, this);
    return true;
}

void SignalEventWatcher::handlerFn(int sn UNUSED, short which UNUSED, void *v) {
    SignalEventWatcher *watcher = (SignalEventWatcher *)v;
    watcher->callback_();
}

} // namespace tair::network
