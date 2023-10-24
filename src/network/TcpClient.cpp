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
#include "network/TcpClient.hpp"

#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"
#include "network/Connector.hpp"
#include "network/EventLoop.hpp"
#include "network/Sockets.hpp"
#include "network/TlsConnection.hpp"

namespace tair::network {

using common::SystemUtil;

TcpClient::TcpClient(EventLoop *loop, std::string remote_ip_port)
    : loop_(loop), remote_ip_port_(std::move(remote_ip_port)), auto_reconnect_(false) {
    LOG_TRACE("TcpClient() this: {}", (void *)this);
}

TcpClient::~TcpClient() {
    if (isConnecting() || isConnected()) {
        LOG_DEBUG("disconnect, this={} remote_addr={}", (void *)this, remote_ip_port_);
        loop_->runInLoop([connector = connector_, conn = connection()](EventLoop *) {
            if (connector->isConnecting()) {
                connector->cancel();
            }
            if (conn) {
                conn->close();
            }
        });
    }
    LOG_TRACE("~TcpClient() this: {}", (void *)this);
}

void TcpClient::connect() {
    if (loop_->isInLoopThread()) {
        connectInLoop();
    } else {
        loop_->queueInLoop([client = shared_from_this()](EventLoop *) {
            client->connectInLoop();
        });
    }
}

void TcpClient::reconnect() {
    LOG_DEBUG("Try to reconnect to {}", remote_ip_port_);
    connect();
}

void TcpClient::disconnect() {
    auto_reconnect_ = false;
    if (loop_->isInLoopThread()) {
        disconnectInLoop();
    } else {
        loop_->queueInLoop([client = shared_from_this()](EventLoop *) {
            client->disconnectInLoop();
        });
    }
}

void TcpClient::connectInLoop() {
    LOG_DEBUG("connect this={} remote_addr={}", (void *)this, remote_ip_port_);
    runtimeAssert(loop_->isInLoopThread());
    connector_ = std::make_shared<Connector>(loop_, remote_ip_port_, connecting_timeout_, auto_reconnect_);
    connector_->setNewConnectionCallback([weak_client = weak_from_this()](socket_t fd, const std::string &local_ip_port, const std::string &remote_ip_port, bool is_tls) {
        auto client = weak_client.lock();
        if (client) {
            client->onNewConnection(fd, local_ip_port, remote_ip_port, is_tls);
        } else {
            LOG_DEBUG("Connector success but client destroyed");
            sockets::closeSocket(fd);
        }
    });
    connector_->start();
}

void TcpClient::disconnectInLoop() {
    LOG_DEBUG("disconnect, this={} remote_addr={}", (void *)this, remote_ip_port_);
    runtimeAssert(loop_->isInLoopThread());
    if (connector_->isConnecting()) {
        LOG_DEBUG("Cancel to connect {}", remote_ip_port_);
        connector_->cancel();
    }
    auto conn = connection();
    if (conn) {
        conn->close();
    }
}

void TcpClient::onNewConnection(socket_t fd, const std::string &local_ip_port, const std::string &remote_ip_port, bool is_tls) {
    runtimeAssert(loop_->isInLoopThread());
    if (fd < 0) {
        LOG_WARN("Failed to connect to {}, error: {} -> {}", remote_ip_port_, errno, SystemUtil::errnoToString(errno));
        // We need to notify this failure event to the user layer
        if (connection_callback_) {
            auto conn = std::make_shared<TcpConnection>(fd, local_ip_port, remote_ip_port);
            conn->setLoop(loop_);
            connection_callback_(conn);
        }
        return;
    }
    if (keepalive_seconds_ > 0) {
        sockets::setKeepAlive(fd, true, keepalive_seconds_);
    }
    LOG_DEBUG("Successfully connected to {}", remote_ip_port_);
    TcpConnectionPtr conn = nullptr;
    if (!is_tls) {
        conn = std::make_shared<TcpConnection>(fd, local_ip_port, remote_ip_port);
    } else {
        conn = std::make_shared<TlsConnection>(fd, local_ip_port, remote_ip_port, TlsConnection::kClient);
    }
    conn->setConnectionCallback(connection_callback_);
    conn->setMessageCallback(message_callback_);
    conn->setWriteCompleteCallback(write_complete_callback_);
    conn->setHighWaterMarkCallback(high_water_mark_callback_, high_water_mark_);
    conn->setCloseCallback(makeWeakCallback<TcpClient>(shared_from_this(), &TcpClient::onCloseConnection));
    {
        LockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->attachedToLoop(loop_);
}

void TcpClient::onCloseConnection(const TcpConnectionPtr &conn) {
    runtimeAssert(loop_->isInLoopThread());
    if (auto_reconnect_) {
        LOG_DEBUG("auto reconnect");
        connectInLoop();
    }
}

bool TcpClient::isConnecting() const {
    return connector_ && connector_->isConnecting();
}

bool TcpClient::isConnected() const {
    LockGuard lock(mutex_);
    return connection_ && connection_->isConnected();
}

TcpConnectionPtr TcpClient::connection() {
    LockGuard lock(mutex_);
    return connection_;
}

} // namespace tair::network
