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

#include <memory>

#include "common/EnableMakeShared.hpp"
#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"
#include "common/WeakCallback.hpp"
#include "network/Duration.hpp"
#include "network/TcpConnection.hpp"
#include "network/Types.hpp"

namespace tair::network {

using common::Mutex;
using common::LockGuard;
using common::Noncopyable;
using common::WeakCallback;
using common::EnableMakeShared;
using common::makeWeakCallback;

class EventLoop;

class TcpClient : private Noncopyable, public std::enable_shared_from_this<TcpClient> {
public:
    static TcpClientPtr create(EventLoop *loop, std::string remote_ip_port) {
        using SharedTcpClient = EnableMakeShared<TcpClient, EventLoop *, std::string>;
        return std::make_shared<SharedTcpClient>(loop, remote_ip_port);
    }

protected:
    explicit TcpClient(EventLoop *loop, std::string remote_ip_port);

public:
    ~TcpClient();

    void connect();
    void reconnect();
    void disconnect();

    void setKeepAlive(int seconds) {
        keepalive_seconds_ = seconds;
    }

    void setConnectingTimeout(Duration timeout) {
        connecting_timeout_ = timeout;
    }

    void setAutoReConnect(bool reconnect) {
        auto_reconnect_ = reconnect;
    }

    void setConnectionCallback(const ConnectionCallback &callback) {
        connection_callback_ = callback;
    }

    void setMessageCallback(const MessageCallback &callback) {
        message_callback_ = callback;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &callback) {
        write_complete_callback_ = callback;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback &callback, size_t mark) {
        high_water_mark_callback_ = callback;
        high_water_mark_ = mark;
    }

    const std::string &remoteIpPort() const {
        return remote_ip_port_;
    }

    bool isConnected() const;

    TcpConnectionPtr connection();

private:
    bool isConnecting() const;
    void connectInLoop();
    void disconnectInLoop();
    void onNewConnection(socket_t fd, const std::string &local_ip_port, const std::string &remote_ip_port, bool is_tls);
    void onCloseConnection(const TcpConnectionPtr &conn);

private:
    EventLoop *loop_;
    std::string remote_ip_port_;
    int keepalive_seconds_ = 0;

    ConnectorPtr connector_;                      // always used in loop thread
    Duration connecting_timeout_ = Duration(3.0); // default 3 seconds
    bool auto_reconnect_;

    mutable Mutex mutex_;
    TcpConnectionPtr connection_ GUARDED_BY(mutex_);

    size_t high_water_mark_ = 128 * 1024 * 1024; // Default 128MB

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    HighWaterMarkCallback high_water_mark_callback_;
};

} // namespace tair::network
