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

#include "network/Duration.hpp"
#include "network/EventWatcher.hpp"
#include "network/Types.hpp"

namespace tair::network {

using common::Noncopyable;

class Timer : Noncopyable {
public:
    Timer(EventLoop *loop, const Callback &callback, const Duration &timeout,
          bool periodic, TimerId id)
        : id_(id) {
        timer_watcher_ = std::make_unique<TimerEventWatcher>(loop, callback, timeout, periodic);
        timer_watcher_->init();
    }

    ~Timer() {
        cancel();
    }

    void start() {
        timer_watcher_->start();
    }

    void cancel() {
        timer_watcher_->cancel();
    }

    TimerId id() const {
        return id_;
    }

    Duration timeout() {
        return timer_watcher_->timeout();
    }

    bool isPeriodic() const {
        return timer_watcher_->isPeriodic();
    }

private:
    TimerId id_;
    std::unique_ptr<TimerEventWatcher> timer_watcher_;
};

} // namespace tair::network
