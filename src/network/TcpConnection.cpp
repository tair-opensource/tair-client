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
#include "network/TcpConnection.hpp"

#include <signal.h>

#include "common/Compiler.hpp"
#include "common/Logger.hpp"
#include "common/StringUtil.hpp"
#include "common/SystemUtil.hpp"
#include "network/Channel.hpp"
#include "network/EventLoop.hpp"
#include "network/NetworkStat.hpp"
#include "network/Sockets.hpp"

namespace {

class IgnoreSigPipe final {
public:
    IgnoreSigPipe() {
        signal(SIGPIPE, SIG_IGN);
    }
    ~IgnoreSigPipe() = default;
};

static IgnoreSigPipe ignoreSigPipe UNUSED;
} // namespace

#define EMPTY_BUFFER_MAX_CAPACITY (1024 * 1024) // 1MB

namespace tair::network {

using common::StringUtil;
using common::SystemUtil;

TcpConnection::TcpConnection(socket_t sockfd, const std::string &local_ip_port, const std::string &remote_ip_port)
    : loop_(nullptr), fd_(sockfd), local_ip_port_(local_ip_port),
      remote_ip_port_(remote_ip_port), status_(kDisconnected),
      input_buffer_(Buffer::INPUT_BUFFER), output_buffer_(Buffer::OUTPUT_BUFFER) {
    auto local_pairs = StringUtil::split(local_ip_port, ':');
    if (local_pairs.size() == 2) {
        local_port_ = std::atoi(local_pairs[1].c_str());
    } else if (local_pairs.size() == 3) {
        local_port_ = std::atoi(local_pairs[2].c_str());
    }
    auto remote_pairs = StringUtil::split(remote_ip_port, ':');
    if (remote_pairs.size() == 2) {
        remote_port_ = std::atoi(remote_pairs[1].c_str());
    }
    // sockfd < 0 means a failed connection, just use for error callback
    if (sockfd > 0) {
        channel_ = std::make_unique<Channel>(sockfd);
        channel_->setReadCallback([this] { handleRead(); });
        channel_->setWriteCallback([this] { handleWrite(); });
    }
    LOG_TRACE("TcpConnection() this: {}", (void *)this);
}

TcpConnection::~TcpConnection() {
    runtimeAssert(status_ == kDisconnected);
    if (fd_ > 0) {
        sockets::closeSocket(fd_);
        fd_ = -1;
        runtimeAssert(channel_->isNoneEvent());
        runtimeAssert(!channel_->isAttached());
    } else {
        runtimeAssert(!channel_);
    }
    LOG_TRACE("~TcpConnection() destroy: {}", (void *)this);
}

void TcpConnection::close() {
    if (status_ == kDisconnecting || status_ == kDisconnected) {
        return;
    }
    status_ = kDisconnecting;
    if (loop_->isInLoopThread()) {
        handleClose();
    } else {
        auto expected_cb = [conn = shared_from_this()]() {
            return conn->loop();
        };
        loop_->queueInLoopMaybeRedir(expected_cb, [conn = shared_from_this()](EventLoop *) {
            conn->handleClose();
        });
    }
}

void TcpConnection::send(std::string &&str) {
    if (status_ != kConnected) {
        LOG_DEBUG("TcpConnection isn't connected, send failed");
        return;
    }
    if (loop_->isInLoopThread()) {
        sendInLoop(str);
    } else {
        auto expected_cb = [conn = shared_from_this()]() {
            return conn->loop();
        };
        loop_->queueInLoopMaybeRedir(expected_cb, [conn = shared_from_this(), mstr = std::move(str)](EventLoop *) {
            conn->sendInLoop(mstr);
        });
    }
}

void TcpConnection::send(Buffer &&buf) {
    if (status_ != kConnected) {
        LOG_DEBUG("TcpConnection isn't connected, send failed");
        return;
    }
    if (loop_->isInLoopThread()) {
        sendInLoop(buf);
    } else {
        auto expected_cb = [conn = shared_from_this()]() {
            return conn->loop();
        };
        loop_->queueInLoopMaybeRedir(expected_cb, [conn = shared_from_this(), mbuf = std::move(buf)](EventLoop *) {
            conn->sendInLoop(mbuf);
        });
    }
}

void TcpConnection::send(const std::string_view &strv) {
    if (status_ != kConnected) {
        LOG_DEBUG("TcpConnection isn't connected, send failed");
        return;
    }
    if (loop_->isInLoopThread()) {
        sendInLoop(strv);
    } else {
        // Avoid cross thread calls, avoid message copy!
        auto str = std::string(strv);
        auto expected_cb = [conn = shared_from_this()]() {
            return conn->loop();
        };
        loop_->queueInLoopMaybeRedir(expected_cb, [conn = shared_from_this(), mstr = std::move(str)](EventLoop *) {
            conn->sendInLoop(mstr);
        });
    }
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
    runtimeAssert(loop_->isInLoopThread());
    if (status_ != kConnected) {
        LOG_WARN("TcpConnection disconnected, give up writing");
        return;
    }
    auto self = shared_from_this();
    if (before_write_event_callback_ && !before_write_event_callback_(self)) {
        return;
    }
    ssize_t nwritten = 0;
    size_t remaining = len;

    // if no data in output queue, writing directly
    if (!channel_->hasWritableEvent() && output_buffer_.empty()) {
        nwritten = sockets::writeToSocket(channel_->fd(), static_cast<const char *>(data), len);
        if (nwritten >= 0) {
            NetworkStat::addNetOutputBytes(nwritten);
            if (after_write_event_callback_) {
                after_write_event_callback_(self, nwritten);
            }
            remaining = len - nwritten;
            if (remaining == 0 && write_complete_callback_) {
                write_complete_callback_(self);
            }
        } else {
            nwritten = 0;
            if (!EVUTIL_ERR_RW_RETRIABLE(errno)) {
                LOG_DEBUG("SendInLoop write failed, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
                if (errno == EPIPE || errno == ECONNRESET) {
                    handleError();
                    return;
                }
            }
        }
    }
    runtimeAssert(remaining <= len);
    if (remaining > 0) {
        size_t old_len = output_buffer_.length();
        size_t current_size = old_len + remaining;
        if (current_size >= high_water_mark_ && old_len < high_water_mark_) {
            LOG_TRACE("Connection high water, current size: {}", current_size);
            if (high_water_mark_callback_) {
                high_water_mark_callback_(self, old_len + remaining);
            }
        }
        output_buffer_.append((char *)data + nwritten, remaining);
        if (!channel_->hasWritableEvent()) {
            channel_->enableWriteEvent();
        }
    }
}

void TcpConnection::sendOutputBuffer() {
    runtimeAssert(loop_->isInLoopThread());
    auto self = shared_from_this();
    if (!output_buffer_.empty() && isConnected()) {
        handleWrite();
        if (!output_buffer_.empty() && isConnected()) {
            if (before_write_event_callback_ && !before_write_event_callback_(self)) {
                return;
            }
            channel_->enableWriteEvent();
        }
    }
}

void TcpConnection::attachedToLoop(EventLoop *loop) {
    runtimeAssert(loop_ == nullptr);
    loop_ = loop;
    runtimeAssert(loop_->isInLoopThread());
    channel_->setLoop(loop_);
    status_ = kConnected;
    channel_->enableReadEvent();
    if (connection_callback_) {
        connection_callback_(shared_from_this());
    }
}

void TcpConnection::moveToNewLoop(EventLoop *new_loop, const Callback &success_cb, const Callback &fail_cb) {
    if (status_ != kConnected) {
        LOG_DEBUG("TcpConnection isn't connected, moveToLoop failed");
        fail_cb();
        return;
    }
    if (loop_->isInLoopThread()) {
        moveToNewLoopInLoop(new_loop, success_cb, fail_cb);
    } else {
        auto expected_cb = [conn = shared_from_this()]() {
            return conn->loop();
        };
        loop_->queueInLoopMaybeRedir(expected_cb, [conn = shared_from_this(), new_loop, success_cb, fail_cb](EventLoop *) {
            conn->moveToNewLoopInLoop(new_loop, success_cb, fail_cb);
        });
    }
}

void TcpConnection::moveToNewLoopInLoop(EventLoop *new_loop, const Callback &success_cb, const Callback &fail_cb) {
    runtimeAssert(loop_->isInLoopThread());
    if (status_ != kConnected) {
        fail_cb();
        return;
    }
    runtimeAssert(loop_ != nullptr);
    detachFromLoopAndReset();
    new_loop->queueInLoop([conn = shared_from_this(), new_loop, success_cb](EventLoop *) {
        runtimeAssert(conn->loop() == nullptr);
        conn->attachToNewLoop(new_loop);
        success_cb();
    });
}

void TcpConnection::detachFromLoopAndReset() {
    LOG_DEBUG("conn({}, fd:{}) detach from loop({}), and reset", (void *)this, fd_, (void *)loop_);
    NetworkStat::addMovingTcpConnCount(1);
    runtimeAssert(NetworkStat::getMovingTcpConnCount() > 0);
    channel_->detachFromLoopAndReset();
    loop_ = nullptr;
}

void TcpConnection::attachToNewLoop(EventLoop *new_loop) {
    loop_ = new_loop;
    LOG_DEBUG("conn({}, fd:{}) attach to new loop({})", (void *)this, fd_, (void *)loop_);
    channel_->attachToNewLoop(new_loop);
    NetworkStat::subMovingTcpConnCount(1);
    runtimeAssert(NetworkStat::getMovingTcpConnCount() >= 0);
}

void TcpConnection::handleRead() {
    runtimeAssert(loop_->isInLoopThread());
    auto self = shared_from_this();
    if (before_read_event_callback_ && !before_read_event_callback_(self)) {
        return;
    }
    int saved_errno = 0;
    if (input_buffer_.empty() && input_buffer_.capacity() > EMPTY_BUFFER_MAX_CAPACITY) {
        input_buffer_.reinit();
    }
    ssize_t nread = input_buffer_.readFromFd(channel_->fd(), &saved_errno);
    if (nread > 0) {
        NetworkStat::addNetInputBytes(nread);
        if (after_read_event_callback_) {
            after_read_event_callback_(self, nread);
        }
        if (message_callback_) {
            message_callback_(self, &input_buffer_);
        } else {
            input_buffer_.reset();
        }
    } else if (nread == 0) {
        handleError();
    } else {
        if (EVUTIL_ERR_RW_RETRIABLE(saved_errno)) {
            LOG_TRACE("read fd error: {} -> {}", saved_errno, SystemUtil::errnoToString(saved_errno));
        } else {
            LOG_DEBUG("read fd error: {} -> {}, closing this connection now", saved_errno, SystemUtil::errnoToString(saved_errno));
            handleError();
        }
    }
}

void TcpConnection::handleWrite() {
    runtimeAssert(loop_->isInLoopThread());
    auto self = shared_from_this();
    if (before_write_event_callback_ && !before_write_event_callback_(self)) {
        return;
    }
    ssize_t nwritten = sockets::writeToSocket(fd_, output_buffer_.data(), output_buffer_.length());
    if (nwritten > 0) {
        NetworkStat::addNetOutputBytes(nwritten);
        output_buffer_.skip(nwritten);
        if (after_write_event_callback_) {
            after_write_event_callback_(self, nwritten);
        }
        if (output_buffer_.empty()) {
            if (output_buffer_.capacity() > EMPTY_BUFFER_MAX_CAPACITY) {
                output_buffer_.reinit();
            }
            channel_->disableWriteEvent();
            if (write_complete_callback_) {
                write_complete_callback_(self);
            }
        }
    } else {
        if (!EVUTIL_ERR_RW_RETRIABLE(errno)) {
            LOG_DEBUG("writeToSocket, addr: {}, error: {} -> {}", remote_ip_port_, errno, SystemUtil::errnoToString(errno));
            if (errno == EPIPE || errno == ECONNRESET) {
                handleError();
            }
        }
    }
}

void TcpConnection::handleClose() {
    runtimeAssert(loop_->isInLoopThread());
    if (status_ == kDisconnected) {
        return;
    }
    LOG_DEBUG("TcpConnection closed, addr={} fd={}", remote_ip_port_, fd_);
    runtimeAssert(status_ == kDisconnecting);
    channel_->closeEvent();
    status_ = kDisconnected;

    TcpConnectionPtr connection(shared_from_this());
    if (connection_callback_) {
        connection_callback_(connection);
    }
    if (close_callback_) {
        close_callback_(connection);
    }
}

void TcpConnection::handleError() {
    status_ = kDisconnecting;
    handleClose();
}

bool TcpConnection::hasReadableEvent() const {
    return channel_->hasReadableEvent();
}

bool TcpConnection::hasWritableEvent() const {
    return channel_->hasWritableEvent();
}

void TcpConnection::enableReadEvent() {
    channel_->enableReadEvent();
}

void TcpConnection::disableReadEvent() {
    channel_->disableReadEvent();
}

void TcpConnection::enableWriteEvent() {
    channel_->enableWriteEvent();
}

void TcpConnection::disableWriteEvent() {
    channel_->disableWriteEvent();
}

std::string TcpConnection::statusToString() const {
    H_CASE_STRING_BIGIN(status_)
    H_CASE_STRING(kDisconnected)
    H_CASE_STRING(kConnecting)
    H_CASE_STRING(kConnected)
    H_CASE_STRING(kDisconnecting)
    H_CASE_STRING_END()
}

} // namespace tair::network
