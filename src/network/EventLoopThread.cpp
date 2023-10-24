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
#include "network/EventLoopThread.hpp"

#include <sstream>

#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"

namespace tair::network {

using common::SystemUtil;

EventLoopThread::EventLoopThread(std::string name, std::string loop_name, int idx)
    : stopped_(true), loop_(nullptr), idx_(idx) {
    if (!name.empty()) {
        name_ = name;
    } else {
        std::stringstream ss;
        ss << "loop-th-" << this;
        name_ = ss.str();
    }
    if (!loop_name.empty()) {
        loop_name_ = loop_name;
    } else {
        std::stringstream ss;
        ss << "loop-of-" << this;
        loop_name_ = ss.str();
    }
}

EventLoopThread::~EventLoopThread() {
    stop();
    join();
}

void EventLoopThread::start() {
    if (stopped_) {
        LOG_DEBUG("EventLoopThread start, name: {}", name_);
        runtimeAssert(!thread_);
        stopped_ = false;
        thread_ = std::make_unique<std::thread>([this]() {
            run();
        });
        {
            UniqueLock lock(mutex_);
            while (!loop_) {
                cond_.wait(lock);
            }
        }
    }
}

void EventLoopThread::stop() {
    if (!stopped_) {
        LOG_DEBUG("EventLoopThread stop, name: {}", name_);
        stopped_ = true;
        {
            LockGuard lock(mutex_);
            if (loop_) {
                loop_->stop();
            }
        }
    }
}

void EventLoopThread::join() {
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

void EventLoopThread::detach() {
    if (thread_) {
        thread_->detach();
    }
}

EventLoop *EventLoopThread::loop() const {
    LockGuard lock(mutex_);
    return loop_;
}

std::thread::id EventLoopThread::tid() const {
    return thread_ ? thread_->get_id() : std::thread::id();
}

void EventLoopThread::run() {
    LOG_DEBUG("EventLoopThread thread run, name: {}", name_);
    SystemUtil::setThreadName(name_);
    EventLoop loop(loop_name_);
    loop.setBeforeSleepCallBack(before_sleep_call_back_);
    loop.setAfterSleepCallBack(after_sleep_call_back_);
    if (init_callback_) {
        LOG_DEBUG("EventLoopThread thread call init callback, name: {}", name_);
        init_callback_(&loop, idx_);
    }
    {
        LockGuard lock(mutex_);
        loop_ = &loop;
    }
    loop.queueInLoop([this](EventLoop *) {
        LockGuard lock(mutex_);
        cond_.notifyOne();
    });
    loop.run();
    {
        LockGuard lock(mutex_);
        loop_ = nullptr;
    }
    // maybe call `detach()` in destroy_callback_, so cache this->name_
    std::string cache_name = name_;
    if (destroy_callback_) {
        LOG_DEBUG("EventLoopThread thread call destroy callback, name: {}", name_);
        destroy_callback_(&loop);
    }
    LOG_DEBUG("EventLoopThread thread exit, name: {}", cache_name);
}

} // namespace tair::network
