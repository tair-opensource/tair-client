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

#include "common/Noncopyable.hpp"
#include "network/Duration.hpp"
#include "network/Types.hpp"

struct event;
struct event_base;

namespace tair::network {

using common::Noncopyable;

class EventLoop;

class EventWatcher : private Noncopyable {
protected:
    EventWatcher(EventLoop *loop, const Callback &callback);

public:
    virtual ~EventWatcher();

    bool init();

    // @note It MUST be called in the event thread
    void cancel();

protected:
    void closeEvent();
    void freeEvent();

    virtual bool doInit() = 0;
    virtual void doClose() {}

    // @note It MUST be called in the event thread
    // @param timeout the maximum amount of time to wait for the event, or 0 to wait forever
    bool attachToEventLoop(Duration timeout);

protected:
    EventLoop *loop_;
    struct event *event_;

    bool attached_;
    bool canceled_;
    Callback callback_;
};

// ---------------------------------------------------------------------------------

class PipeEventWatcher final : public EventWatcher {
public:
    PipeEventWatcher(EventLoop *loop, const Callback &callback);
    ~PipeEventWatcher() override;

    bool start();
    void notify();

private:
    bool doInit() override;
    void doClose() override;
    static void handlerFn(int fd, short which, void *v);

    socket_t pipe_[2] = {-1, -1}; // write to pipe_[0] , read from pipe_[1]
};

// ---------------------------------------------------------------------------------

class TimerEventWatcher final : public EventWatcher {
public:
    TimerEventWatcher(EventLoop *loop, const Callback &callback, Duration timeout, bool periodic);

    bool start();

    Duration timeout() {
        return timeout_;
    }

    bool isPeriodic() const {
        return periodic_;
    }

private:
    bool doInit() override;
    static void handlerFn(int fd, short which, void *v);

private:
    Duration timeout_;
    bool periodic_;
};

// ---------------------------------------------------------------------------------

class SignalEventWatcher final : public EventWatcher {
public:
    SignalEventWatcher(int signo, EventLoop *loop, const Callback &callback);

    bool start();

private:
    bool doInit() override;
    static void handlerFn(int sn, short which, void *v);

private:
    int signo_ = 0;
};

} // namespace tair::network
