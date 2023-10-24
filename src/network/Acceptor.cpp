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
#include "network/Acceptor.hpp"

#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"
#include "network/Channel.hpp"
#include "network/EventLoop.hpp"
#include "network/Sockets.hpp"

namespace tair::network {

using common::SystemUtil;

Acceptor::Acceptor(EventLoop *loop, const std::string &listen_ip_port)
    : loop_(loop), listen_fd_(-1) {
    ori_listen_ip_port_ = listen_ip_port;
    if (listen_ip_port.starts_with("tcp://") || listen_ip_port.starts_with("tls://")) {
        if (listen_ip_port.starts_with("tls://")) {
            is_tls_ = true;
        }
        listen_ip_port_ = listen_ip_port.substr(6);
    }
}

Acceptor::~Acceptor() {
    channel_.reset();
    if (listen_fd_ != -1) {
        sockets::closeSocket(listen_fd_);
        listen_fd_ = -1;
    }
}

bool Acceptor::listen() {
    listen_fd_ = sockets::createNonblockingSocket();
    if (listen_fd_ < 0) {
        LOG_ERROR("create a nonblocking socket failed, errno: {}-> {}", errno, SystemUtil::errnoToString(errno));
        return false;
    }
    struct sockaddr_storage addr = sockets::parseFromIPPort(listen_ip_port_.data());
    if (sockets::isZeroAddress(&addr)) {
        return false;
    }
    int ret = sockets::bindSocket(listen_fd_, sockets::sockaddr_cast(&addr), sizeof(struct sockaddr));
    if (ret < 0) {
        LOG_ERROR("bind failed, errno: {} -> {}, addr={}", errno, SystemUtil::errnoToString(errno), listen_ip_port_);
        return false;
    }
    if (sockets::sockaddr_in_cast(&addr)->sin_port == 0) {
        struct sockaddr_storage local_addr = sockets::getLocalAddr(listen_fd_);
        real_listen_ip_port_ = sockets::toIPPort(&local_addr);
        LOG_INFO("listen port is 0, real listen addr is: {}", real_listen_ip_port_);
    } else {
        real_listen_ip_port_ = listen_ip_port_;
    }
    ret = sockets::listenSocket(listen_fd_);
    if (ret < 0) {
        LOG_ERROR("listen failed, errno: {} -> {}, addr={}", errno, SystemUtil::errnoToString(errno), real_listen_ip_port_);
        return false;
    }
    return true;
}

void Acceptor::startAccept() {
    runtimeAssert(new_conn_callback_);
    channel_ = std::make_shared<Channel>(listen_fd_);
    channel_->setLoop(loop_);
    channel_->setReadCallback([this]() {
        handleAccept();
    });
    loop_->runInLoop([acceptor = shared_from_this()](EventLoop *) {
        acceptor->channel_->enableReadEvent();
    });
    LOG_DEBUG("Acceptor is running at: {}", real_listen_ip_port_);
}

void Acceptor::stop() {
    loop_->runInLoop([acceptor = shared_from_this()](EventLoop *) {
        LOG_DEBUG("Acceptor is stopping at: {}", acceptor->real_listen_ip_port_);
        acceptor->channel_->closeEvent();
    });
}

void Acceptor::handleAccept() {
    runtimeAssert(loop_->isInLoopThread());
    struct sockaddr_storage ss;
    ::memset(&ss, 0, sizeof(ss));
    socklen_t addrlen = sizeof(ss);
    int nfd = -1;
    if ((nfd = sockets::acceptSocket(listen_fd_, sockets::sockaddr_cast(&ss), &addrlen)) == -1) {
        if (errno != EAGAIN && errno != EINTR) {
            LOG_WARN("bad accept, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
        }
        return;
    }
    if (sockets::setNonBlocking(nfd) < 0) {
        LOG_ERROR("set fd={} nonblocking failed, close it", nfd);
        sockets::closeSocket(nfd);
        return;
    }
    sockets::setTcpNoDelay(nfd, true);

    std::string remote_ip_port = sockets::toIPPort(&ss);
    if (remote_ip_port.empty()) {
        LOG_ERROR("sock::ToIPPort(&ss) failed, close it");
        sockets::closeSocket(nfd);
        return;
    }
    LOG_TRACE("accepted a connection from {}, listen fd={}, client fd={}", remote_ip_port, listen_fd_, nfd);

    new_conn_callback_(nfd, real_listen_ip_port_, remote_ip_port, is_tls_);
}

} // namespace tair::network
