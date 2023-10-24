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
#include <sys/socket.h>

#include "common/Noncopyable.hpp"
#include "network/Duration.hpp"
#include "network/Types.hpp"

namespace tair::network {

using common::Noncopyable;

class EventLoop;
class Channel;
class TimerEventWatcher;
class TcpClient;

class Connector final : private Noncopyable, public std::enable_shared_from_this<Connector> {
public:
    Connector(EventLoop *loop, const std::string &remote_ip_port, Duration connecting_timeout, bool need_retry);
    ~Connector();

    void start();
    void cancel();

    void setNewConnectionCallback(const NewConnectionCallback &callback) {
        new_conn_callback_ = callback;
    }

    bool isConnecting() const {
        return status_ == kConnecting;
    }

private:
    void connect();
    void handleWrite();
    void handleError();
    void onConnectTimeout();
    void closeTimer();
    void closeChannel();
    void closeFd();
    std::string statusToString() const;

private:
    enum Status { kDisconnected,
                  kConnecting,
                  kConnected };

    static const Duration kInitRetryDelayTime;
    static const Duration kMaxRetryDelayTime;

    Status status_;
    EventLoop *loop_;

    bool is_tls_ = false;
    std::string local_ip_port_;
    std::string remote_ip_port_;
    struct sockaddr_storage remote_sockaddr_;

    TimerId connecting_timer_id_;
    Duration connecting_timeout_;
    socket_t fd_;

    // A flag indicate whether the Connector owns this fd
    // If the Connector owns this fd, the Connector has responsibility to close this fd
    bool own_fd_;

    bool need_retry_;
    Duration retry_delay_time_;

    std::shared_ptr<Channel> channel_;
    NewConnectionCallback new_conn_callback_;
};

} // namespace tair::network
