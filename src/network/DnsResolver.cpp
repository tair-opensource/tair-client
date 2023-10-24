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
#include "network/DnsResolver.hpp"

#include "common/Logger.hpp"
#include "network/EventLoop.hpp"
#include "network/Sockets.hpp"

namespace tair::network {

DnsResolver::DnsResolver(EventLoop *loop, std::string host,
                         Duration timeout, const DnsResolverCallback &callback)
    : stopped_(true), loop_(loop), host_(host), timeout_(timeout), resolver_callback_(callback), resolver_timer_id_(0) {
}

DnsResolver::~DnsResolver() {
    runtimeAssert(stopped_);
    runtimeAssert(resolver_timer_id_ == 0);
    runtimeAssert(!dns_base_);
    runtimeAssert(!dns_req_);
}

void DnsResolver::start() {
    runtimeAssert(stopped_);
    stopped_ = false;
    if (loop_->isInLoopThread()) {
        startInLoop();
    } else {
        loop_->queueInLoop([resolver = shared_from_this()](EventLoop *) {
            resolver->startInLoop();
        });
    }
}

void DnsResolver::onTimeout() {
    if (stopped_) {
        runtimeAssert(!dns_base_);
        runtimeAssert(!dns_req_);
        return;
    }
    stopped_ = true;
    LOG_TRACE("dns resolver timeout, tid={} this={}", std::this_thread::get_id(), (void *)this);
    runtimeAssert(loop_->isInLoopThread());

    // timer auto removed by loop, just set id = 0
    runtimeAssert(resolver_timer_id_ != 0);
    resolver_timer_id_ = 0;

    if (dns_req_) {
        evdns_getaddrinfo_cancel(dns_req_);
        dns_req_ = nullptr;
    }
    clearDnsBase();
}

void DnsResolver::clearTimer() {
    if (resolver_timer_id_ != 0) {
        loop_->cancelTimer(resolver_timer_id_);
        resolver_timer_id_ = 0;
    }
}

void DnsResolver::clearDnsBase() {
    if (dns_base_) {
        evdns_base_free(dns_base_, 0);
        dns_base_ = nullptr;
    }
}

void DnsResolver::onResolved(int errcode) {
    if (resolver_callback_) {
        resolver_callback_(errcode, addrs_);
    }
}

void DnsResolver::onResolved(int errcode, struct addrinfo *addr) {
    runtimeAssert(loop_->isInLoopThread());
    clearTimer();
    if (!stopped_) {
        stopped_ = true;
        clearDnsBase();
    } else {
        runtimeAssert(resolver_timer_id_ == 0);
        runtimeAssert(!dns_base_);
    }
    // libevent already free it
    dns_req_ = nullptr;
    if (errcode == 0 && addr) {
        for (struct addrinfo *rp = addr; rp != nullptr; rp = rp->ai_next) {
            struct sockaddr_in *a = sockets::sockaddr_in_cast(rp->ai_addr);
            if (a->sin_addr.s_addr == 0) {
                continue;
            }
            std::string ip = sockets::toIP(sockets::sockaddr_cast(a));
            LOG_TRACE("host={} resolved a ip={}", host_, ip);
            addrs_.emplace_back(ip);
        }
        evutil_freeaddrinfo(addr);
    } else {
        if (errcode != EVUTIL_EAI_CANCEL) {
            LOG_ERROR("DNS resolve failed, error code: {}, error msg: {}", errcode, evutil_gai_strerror(errcode));
        } else {
            LOG_WARN("DNS resolve cancel, may be timeout");
        }
    }
    onResolved(errcode);
}

void DnsResolver::onDnsResolved(int errcode, struct addrinfo *addr, void *arg) {
    // ugly code
    std::shared_ptr<DnsResolver> *pshared = reinterpret_cast<std::shared_ptr<DnsResolver> *>(arg);
    (*pshared)->onResolved(errcode, addr);
    delete pshared;
}

void DnsResolver::startInLoop() {
    runtimeAssert(loop_->isInLoopThread());
    runtimeAssert(!dns_base_);
    runtimeAssert(!dns_req_);
    runtimeAssert(resolver_timer_id_ == 0);

    struct addrinfo hints;
    ::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP; // use TCP socket
    hints.ai_flags = AI_PASSIVE;     // for wildcard IP address

    resolver_timer_id_ = loop_->runAfterTimer(timeout_, [resolver = shared_from_this()](EventLoop *) {
        resolver->onTimeout();
    });

    // ugly code
    auto *pshared = new std::shared_ptr<DnsResolver>(shared_from_this());
    dns_base_ = evdns_base_new(loop_->getEventBase(), 1);
    dns_req_ = evdns_getaddrinfo(dns_base_, host_.c_str(), nullptr, &hints, &DnsResolver::onDnsResolved, pshared);
}

} // namespace tair::network
