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

#include "network/EventLoop.hpp"

struct event_base;

namespace tair::network {

using common::Noncopyable;
using common::Mutex;
using common::LockGuard;
using common::Condition;
using common::UniqueLock;

class EventLoop;

class EventLoopThread final : private Noncopyable {
public:
    EventLoopThread(std::string name = "", std::string loop_name = "", int idx = -1);
    ~EventLoopThread();

    void start();
    void stop();
    void join();
    void detach();

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

    const std::string &name() const {
        return name_;
    }

    EventLoop *loop() const;

    std::thread::id tid() const;

    bool isRunning() const {
        return !stopped_;
    }

private:
    void run();

private:
    std::atomic<bool> stopped_;

    mutable Mutex mutex_;
    Condition cond_ GUARDED_BY(mutex_);
    EventLoop *loop_ GUARDED_BY(mutex_);

    EventLoopInitCallback init_callback_;
    EventLoopDestroyCallback destroy_callback_;
    EventLoopBeforeSleepCallBack before_sleep_call_back_;
    EventLoopAfterSleepCallBack after_sleep_call_back_;

    std::unique_ptr<std::thread> thread_;

    std::string name_;
    std::string loop_name_;
    int idx_ = -1;
};

} // namespace tair::network
