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
#include "network/Channel.hpp"

#include "common/Logger.hpp"
#include "network/EventLoop.hpp"
#include "network/Sockets.hpp"

namespace tair::network {

static_assert(Channel::kReadable == EV_READ, "Channel::kReadable != EV_READ");
static_assert(Channel::kWritable == EV_WRITE, "Channel::kWritable != EV_WRITE");

Channel::Channel(socket_t fd)
    : attached_(false), loop_(nullptr), events_(kNone), event_(nullptr), fd_(fd) {
    runtimeAssert(fd > 0);
    event_ = new event;
    ::memset(event_, 0, sizeof(struct event));
}

Channel::~Channel() {
    runtimeAssert(events_ == kNone);
    runtimeAssert(!attached_);
    runtimeAssert(!event_);
}

void Channel::closeEvent() {
    runtimeAssert(loop_->isInLoopThread());
    if (event_) {
        if (attached_) {
            ::event_del(event_);
            events_ = kNone;
            attached_ = false;
        }
        delete event_;
        event_ = nullptr;
        read_callback_ = nullptr;
        write_callback_ = nullptr;
    }
}

void Channel::attachToLoop() {
    runtimeAssert(loop_->isInLoopThread());
    runtimeAssert(!isNoneEvent());
    if (attached_) {
        // FdChannel::Update may be called many times
        // So doing this can avoid event_add will be called more than once.
        detachFromLoop();
    }
    runtimeAssert(!attached_);
    // fd event always add EV_PERSIST flag
    ::event_set(event_, fd_, events_ | EV_PERSIST, &Channel::handleEvent, this);
    ::event_base_set(loop_->getEventBase(), event_);

    if (::event_add(event_, nullptr) == 0) {
        LOG_DEBUG("fd={} watching event {}", fd_, eventsToString());
        attached_ = true;
    } else {
        LOG_ERROR("attachToLoop this={} fd={} with event {} attach to event loop failed", (void *)this, fd_, eventsToString());
    }
}

void Channel::detachFromLoop() {
    runtimeAssert(loop_->isInLoopThread());
    runtimeAssert(attached_);
    if (::event_del(event_) == 0) {
        LOG_DEBUG("fd={} detach from event loop", fd_);
        attached_ = false;
    } else {
        LOG_ERROR("detachFromLoop this={} fd={} with event {} detach to event loop failed", (void *)this, fd_, eventsToString());
    }
}

void Channel::detachFromLoopAndReset() {
    detachFromLoop();
    loop_ = nullptr;
}

void Channel::attachToNewLoop(EventLoop *loop) {
    loop_ = loop;
    attachToLoop();
}

void Channel::enableReadEvent() {
    int events = events_;
    events_ |= kReadable;
    if (events_ != events) {
        updateEvent();
    }
}

void Channel::enableWriteEvent() {
    int events = events_;
    events_ |= kWritable;
    if (events_ != events) {
        updateEvent();
    }
}

void Channel::disableReadEvent() {
    int events = events_;
    events_ &= ~kReadable;
    if (events_ != events) {
        updateEvent();
    }
}

void Channel::disableWriteEvent() {
    int events = events_;
    events_ &= ~kWritable;
    if (events_ != events) {
        updateEvent();
    }
}

void Channel::disableAllEvent() {
    if (events_ != kNone) {
        events_ = kNone;
        updateEvent();
    }
}

std::string Channel::eventsToString() const {
    std::string str;
    if (events_ & kReadable) {
        str = "kReadable";
    }
    if (events_ & kWritable) {
        if (!str.empty()) {
            str += "|";
        }
        str += "kWritable";
    }
    return str;
}

void Channel::handleEvent(socket_t fd, short which) {
    runtimeAssert(fd == fd_);
    // maybe release channel after read_callback_
    auto guard = shared_from_this();
    if ((which & kReadable) && attached_ && read_callback_) {
        read_callback_();
    }
    if ((which & kWritable) && attached_ && write_callback_) {
        write_callback_();
    }
}

void Channel::handleEvent(socket_t fd, short which, void *v) {
    Channel *c = (Channel *)v;
    c->handleEvent(fd, which);
}

void Channel::updateEvent() {
    runtimeAssert(loop_->isInLoopThread());
    if (isNoneEvent()) {
        detachFromLoop();
    } else {
        attachToLoop();
    }
}

} // namespace tair::network
