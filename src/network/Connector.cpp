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
#include "network/Connector.hpp"

#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"
#include "network/Channel.hpp"
#include "network/EventLoop.hpp"
#include "network/EventWatcher.hpp"
#include "network/Sockets.hpp"

namespace tair::network {

using common::SystemUtil;

const Duration Connector::kInitRetryDelayTime = Duration(500 * Duration::kMillisecond);
const Duration Connector::kMaxRetryDelayTime = Duration(30 * Duration::kSecond);

Connector::Connector(EventLoop *loop, const std::string &remote_ip_port, Duration connecting_timeout, bool need_retry)
    : status_(kDisconnected),
      loop_(loop),
      remote_ip_port_(remote_ip_port),
      connecting_timer_id_(0),
      connecting_timeout_(connecting_timeout),
      fd_(-1),
      own_fd_(false),
      need_retry_(need_retry),
      retry_delay_time_(kInitRetryDelayTime) {
    if (remote_ip_port.starts_with("tcp://") || remote_ip_port.starts_with("tls://")) {
        if (remote_ip_port.starts_with("tls://")) {
            is_tls_ = true;
        }
        remote_ip_port_ = remote_ip_port.substr(6);
    }
    remote_sockaddr_ = sockets::parseFromIPPort(remote_ip_port_.data());
    LOG_TRACE("Connector() this: {}", (void *)this);
}

Connector::~Connector() {
    runtimeAssert(connecting_timer_id_ == 0);
    runtimeAssert(!channel_ || channel_->isNoneEvent());
    runtimeAssert(!own_fd_ && fd_ == -1);
    LOG_TRACE("~Connector() this: {}", (void *)this);
}

void Connector::start() {
    runtimeAssert(loop_->isInLoopThread());
    if (!sockets::isZeroAddress(&remote_sockaddr_)) {
        connect();
    } else {
        LOG_ERROR("Connector start failed, connect to wrong addr?");
        need_retry_ = false;
        handleError();
    }
}

void Connector::cancel() {
    runtimeAssert(loop_->isInLoopThread());
    if (status_ != kConnecting) {
        return;
    }
    need_retry_ = false;
    status_ = kDisconnected;
    closeTimer();
    closeChannel();
    closeFd();
}

void Connector::connect() {
    runtimeAssert(loop_->isInLoopThread());
    runtimeAssert(fd_ == -1 && !own_fd_);
    fd_ = sockets::createNonblockingSocket();
    if (fd_ < 0) {
        LOG_ERROR("create a nonblocking socket failed, errno: {}-> {}", errno, SystemUtil::errnoToString(errno));
        handleError();
        return;
    }
    own_fd_ = true;

    // add connecting timeout timer
    auto timeout_handler = [connector = shared_from_this()](EventLoop *) {
        connector->onConnectTimeout();
    };
    connecting_timer_id_ = loop_->runAfterTimer(connecting_timeout_, timeout_handler);

    struct sockaddr *addr = sockets::sockaddr_cast(&remote_sockaddr_);
    int rc = sockets::connectSocket(fd_, addr, sizeof(*addr));
    if (rc != 0) {
        if (!EVUTIL_ERR_CONNECT_RETRIABLE(errno)) {
            handleError();
            return;
        }
    }
    status_ = kConnecting;
    channel_ = std::make_shared<Channel>(fd_);
    channel_->setLoop(loop_);
    channel_->setWriteCallback([connector = shared_from_this()]() {
        connector->handleWrite();
    });
    channel_->enableWriteEvent();
}

void Connector::handleWrite() {
    if (status_ == kDisconnected) {
        return;
    }
    runtimeAssert(status_ == kConnecting);
    runtimeAssert(fd_ == channel_->fd());
    int err = sockets::getSocketErrorCode(fd_);
    if (err != 0) {
        EVUTIL_SET_SOCKET_ERROR(err);
        handleError();
        return;
    }
    LOG_TRACE("Connector success, now cancel timer and close write events");
    status_ = kConnected;
    closeTimer();

    // close channel will reset callback in channel, so we need hold myself
    // Avoid that the client has been deconstructed and myself will be deconstructed
    auto hold_myself = shared_from_this();
    closeChannel();

    struct sockaddr_storage addr = sockets::getLocalAddr(fd_);
    local_ip_port_ = sockets::toIPPort(&addr);
    new_conn_callback_(fd_, local_ip_port_, remote_ip_port_, is_tls_);

    own_fd_ = false; // move the ownership of the fd to TcpConnection
    fd_ = -1;
}

void Connector::handleError() {
    runtimeAssert(loop_->isInLoopThread());
    status_ = kDisconnected;

    int saved_errno = errno;
    LOG_ERROR("Connector error, status={} fd={} errno={} -> {}", statusToString(), fd_, saved_errno, SystemUtil::errnoToString(saved_errno));
    closeTimer();

    // close channel will reset callback in channel, so we need hold myself
    // Avoid that the client has been deconstructed and myself will be deconstructed
    auto hold_myself = shared_from_this();
    closeChannel();

    closeFd();

    if (EVUTIL_ERR_CONNECT_REFUSED(saved_errno) || !need_retry_) {
        // notify error
        new_conn_callback_(-1, "", remote_ip_port_, is_tls_);
    }

    if (need_retry_) {
        loop_->runAfterTimer(retry_delay_time_, [connector = shared_from_this()](EventLoop *) {
            connector->connect();
        });
        retry_delay_time_ *= 2;
        retry_delay_time_ = std::min(retry_delay_time_, kMaxRetryDelayTime);
    }
}

void Connector::onConnectTimeout() {
    if (status_ == kConnected) {
        return;
    }
    LOG_WARN("Connector::OnConnectTimeout status={} fd={} this={}", statusToString(), fd_, (void *)this);
    runtimeAssert(status_ == kConnecting);

    // timer auto removed by loop, just set id = 0
    runtimeAssert(connecting_timer_id_ != 0);
    connecting_timer_id_ = 0;

    EVUTIL_SET_SOCKET_ERROR(ETIMEDOUT);
    handleError();
}

void Connector::closeTimer() {
    if (connecting_timer_id_ != 0) {
        loop_->cancelTimer(connecting_timer_id_);
        connecting_timer_id_ = 0;
    }
}

void Connector::closeChannel() {
    if (channel_) {
        runtimeAssert(!channel_->hasReadableEvent());
        runtimeAssert(channel_->hasWritableEvent());
        channel_->closeEvent();
        channel_.reset();
    }
}

void Connector::closeFd() {
    if (own_fd_ && fd_ != -1) {
        sockets::closeSocket(fd_);
        fd_ = -1;
        own_fd_ = false;
    }
}

std::string Connector::statusToString() const {
    H_CASE_STRING_BIGIN(status_)
    H_CASE_STRING(kDisconnected)
    H_CASE_STRING(kConnecting)
    H_CASE_STRING(kConnected)
    H_CASE_STRING_END()
}

} // namespace tair::network
