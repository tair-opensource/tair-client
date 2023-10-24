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
#include "client/TairURI.hpp"

namespace tair::client {

TairURIBuilder TairURI::create() {
    return TairURIBuilder();
}

TairURI::ConnectType TairURI::getType() const {
    return type_;
}

std::string TairURI::typeToString() const {
    switch (type_) {
        case STANDALONE:
            return "standalone";
        case CLUSTER:
            return "cluster";
        case SENTINEL:
            return "sentinel";
        default:
            return "unknown";
    }
}

const std::vector<std::string> &TairURI::getServerAddrs() const {
    return server_addrs_;
}

int TairURI::getConnectingTimeoutMs() const {
    return connecting_timeout_ms_;
}

int TairURI::getReconnectIntervalMs() const {
    return reconnect_interval_ms_;
}

int TairURI::getKeepAliveSeconds() const {
    return keepalive_seconds_;
}

const std::string &TairURI::getUser() const {
    return user_;
}

const std::string &TairURI::getPassword() const {
    return password_;
}

bool TairURI::isAutoReconnect() const {
    return auto_reconnect_;
}

EventLoop *TairURI::getLoop() const {
    return loop_;
}

TairURI &TairURIBuilder::build() {
    return uri_;
}

TairURIBuilder &TairURIBuilder::type(TairURI::ConnectType type) {
    uri_.type_ = type;
    return *this;
}

TairURIBuilder &TairURIBuilder::serverAddrs(std::vector<std::string> server_addrs) {
    uri_.server_addrs_ = std::move(server_addrs);
    return *this;
}

TairURIBuilder &TairURIBuilder::connectingTimeoutMs(int timeout_ms) {
    uri_.connecting_timeout_ms_ = timeout_ms;
    return *this;
}

TairURIBuilder &TairURIBuilder::reconnectIntervalMs(int timeout_ms) {
    uri_.reconnect_interval_ms_ = timeout_ms;
    return *this;
}

TairURIBuilder &TairURIBuilder::keepalive(int seconds) {
    uri_.keepalive_seconds_ = seconds;
    return *this;
}

TairURIBuilder &TairURIBuilder::autoReconnect(bool reconnect) {
    uri_.auto_reconnect_ = reconnect;
    return *this;
}

TairURIBuilder &TairURIBuilder::user(std::string user) {
    uri_.user_ = std::move(user);
    return *this;
}

TairURIBuilder &TairURIBuilder::password(std::string password) {
    uri_.password_ = std::move(password);
    return *this;
}

TairURIBuilder &TairURIBuilder::eventloop(EventLoop *loop) {
    uri_.loop_ = loop;
    return *this;
}

} // namespace tair::client