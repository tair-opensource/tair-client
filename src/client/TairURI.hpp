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
#include <vector>

namespace tair::network {
class EventLoop;
} // namespace tair::network

namespace tair::client {

using network::EventLoop;

class TairURIBuilder;

class TairURI {
public:
    friend class TairURIBuilder;
    enum ConnectType { STANDALONE,
                       CLUSTER,
                       SENTINEL };
    static TairURIBuilder create();

    ConnectType getType() const;
    const std::vector<std::string> &getServerAddrs() const;
    int getConnectingTimeoutMs() const;
    int getReconnectIntervalMs() const;
    int getKeepAliveSeconds() const;
    bool isAutoReconnect() const;
    const std::string &getUser() const;
    const std::string &getPassword() const;
    EventLoop *getLoop() const;

private:
    TairURI() = default;
    std::string typeToString() const;

    ConnectType type_ = STANDALONE;
    std::vector<std::string> server_addrs_;
    int connecting_timeout_ms_ = 2000;
    int reconnect_interval_ms_ = -1;
    int keepalive_seconds_ = 60;
    bool auto_reconnect_ = true;
    std::string user_;
    std::string password_;
    EventLoop *loop_ = nullptr;
};

class TairURIBuilder {
public:
    TairURIBuilder() = default;
    ~TairURIBuilder() = default;
    TairURI &build();

    TairURIBuilder &type(TairURI::ConnectType type);
    TairURIBuilder &serverAddrs(std::vector<std::string> server_addrs);
    TairURIBuilder &connectingTimeoutMs(int timeout_ms);
    TairURIBuilder &reconnectIntervalMs(int timeout_ms);
    TairURIBuilder &keepalive(int seconds);
    TairURIBuilder &autoReconnect(bool reconnect);
    TairURIBuilder &user(std::string user);
    TairURIBuilder &password(std::string password);
    TairURIBuilder &eventloop(EventLoop *loop);

private:
    TairURI uri_;
};

} // namespace tair::client
