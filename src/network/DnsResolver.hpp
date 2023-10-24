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

#include <atomic>
#include <string>
#include <vector>

#include "common/EnableMakeShared.hpp"
#include "common/Noncopyable.hpp"
#include "network/Duration.hpp"
#include "network/EventWatcher.hpp"
#include "network/Types.hpp"

struct evdns_base;
struct evdns_getaddrinfo_request;
struct addrinfo;

namespace tair::network {

using common::Noncopyable;
using common::EnableMakeShared;

class EventLoop;

class DnsResolver : private Noncopyable, public std::enable_shared_from_this<DnsResolver> {
public:
    using DnsResolverCallback = std::function<void(int errcode, const std::vector<std::string> &addrs)>;
    using SharedDnsResolver = EnableMakeShared<DnsResolver, EventLoop *, std::string, Duration, const DnsResolverCallback &>;

    static DnsResolverPtr createDnsResolver(EventLoop *loop, std::string host,
                                            Duration timeout, const DnsResolverCallback &callback) {
        return std::make_shared<SharedDnsResolver>(loop, host, timeout, callback);
    }

protected:
    DnsResolver(EventLoop *loop, std::string host, Duration timeout, const DnsResolverCallback &callback);

public:
    ~DnsResolver();

    void start();

private:
    void clearTimer();
    void clearDnsBase();
    void onTimeout();
    void onResolved(int errcode);
    void onResolved(int errcode, struct addrinfo *addr);
    static void onDnsResolved(int errcode, struct addrinfo *addr, void *arg);
    void startInLoop();

private:
    std::atomic<bool> stopped_;
    EventLoop *loop_;
    struct evdns_base *dns_base_ = nullptr;
    struct evdns_getaddrinfo_request *dns_req_ = nullptr;
    std::string host_;
    Duration timeout_;
    DnsResolverCallback resolver_callback_;
    TimerId resolver_timer_id_;
    std::vector<std::string> addrs_;
};

} // namespace tair::network
