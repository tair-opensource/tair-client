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
#include <string>

#include "common/Noncopyable.hpp"
#include "network/Types.hpp"

namespace tair::network {

class EventLoop;
class Channel;

using common::Noncopyable;

class Acceptor final : Noncopyable, public std::enable_shared_from_this<Acceptor> {
public:
    Acceptor(EventLoop *loop, const std::string &listen_ip_port);
    virtual ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &callback) {
        new_conn_callback_ = callback;
    }

    bool listen();
    void startAccept();
    void stop();

    const std::string &getOriListenIpPort() const {
        return ori_listen_ip_port_;
    }

    const std::string &getListenIpPort() const {
        return listen_ip_port_;
    }

    const std::string &getRealListenIpPort() const {
        return real_listen_ip_port_;
    }

private:
    void handleAccept();

private:
    EventLoop *loop_;
    std::string ori_listen_ip_port_;
    std::string listen_ip_port_;
    std::string real_listen_ip_port_;
    bool is_tls_ = false;
    socket_t listen_fd_;
    std::shared_ptr<Channel> channel_;
    NewConnectionCallback new_conn_callback_;
};

} // namespace tair::network
