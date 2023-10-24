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

#include <string>

#include "common/Noncopyable.hpp"
#include "network/Types.hpp"

struct event;

namespace tair::network {

using common::Noncopyable;

class EventLoop;

class Channel final : public std::enable_shared_from_this<Channel>, private Noncopyable {
public:
    enum EventType {
        kNone = 0x00,
        kReadable = 0x02, // same to EV_READ in libevent
        kWritable = 0x04, // same to EV_WRITE in libevent
    };

public:
    Channel(socket_t fd);
    ~Channel();

    void setLoop(EventLoop *loop) {
        loop_ = loop;
    }

    void closeEvent();

    inline bool isAttached() const {
        return attached_;
    }

    inline bool hasReadableEvent() const {
        return (events_ & kReadable) != 0;
    }

    inline bool hasWritableEvent() const {
        return (events_ & kWritable) != 0;
    }

    inline bool isNoneEvent() const {
        return events_ == kNone;
    }

    // for TcpConnection move to other EventLoop
    // MUST call in loop thread
    void detachFromLoopAndReset();
    void attachToNewLoop(EventLoop *loop);

    void enableReadEvent();
    void enableWriteEvent();

    void disableReadEvent();
    void disableWriteEvent();

    void disableAllEvent();

    inline socket_t fd() const {
        return fd_;
    }

    std::string eventsToString() const;

    inline void setReadCallback(const Callback &cb) {
        read_callback_ = cb;
    }

    inline void setWriteCallback(const Callback &cb) {
        write_callback_ = cb;
    }

private:
    void handleEvent(socket_t fd, short which);
    static void handleEvent(socket_t fd, short which, void *v);

    void updateEvent();
    void attachToLoop();
    void detachFromLoop();

private:
    Callback read_callback_;
    Callback write_callback_;

    bool attached_;
    EventLoop *loop_;

    int events_;
    struct event *event_;

    socket_t fd_;
};

} // namespace tair::network
