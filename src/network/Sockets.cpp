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
#include "network/Sockets.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include "common/Assert.hpp"
#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"

namespace tair::network {
namespace sockets {

using common::SystemUtil;

int createSocketPair(int domain, int type, int protocol, socket_t sock[2]) {
    return ::evutil_socketpair(domain, type, protocol, sock);
}

socket_t createNonblockingSocket() {
    socket_t fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    if (setNonBlocking(fd) < 0 || setCloseOnExec(fd) < 0) {
        closeSocket(fd);
        return -1;
    }
    setTcpNoDelay(fd, true);

    setReuseAddr(fd);
    // setReusePort(fd);

    return fd;
}

int bindSocket(socket_t fd, const struct sockaddr *addr, socklen_t addrlen) {
    return ::bind(fd, addr, addrlen);
}

int listenSocket(socket_t fd, int backlog) {
    return ::listen(fd, backlog);
}

int acceptSocket(socket_t fd, struct sockaddr *addr, socklen_t *addrlen) {
    return ::accept(fd, addr, addrlen);
}

int connectSocket(socket_t fd, const struct sockaddr *addr, socklen_t len) {
    return ::connect(fd, addr, len);
}

void closeSocket(socket_t fd) {
    ::evutil_closesocket(fd);
}

int readVFromSocket(socket_t fd, const struct iovec *vec, int iovcnt) {
    return ::readv(fd, vec, iovcnt);
}

int readFromSocket(socket_t fd, void *buf, int len) {
    return ::read(fd, buf, len);
}

int writeToSocket(socket_t fd, const void *buf, size_t len) {
    return ::write(fd, buf, len);
}

int getSocketErrorCode(socket_t fd) {
    int status = 0;
    socklen_t slen = sizeof(status);
    int ret = ::getsockopt(fd, SOL_SOCKET, SO_ERROR, &status, &slen);
    return ret < 0 ? errno : status;
}

int setNonBlocking(socket_t fd) {
    return ::evutil_make_socket_nonblocking(fd);
}

int setCloseOnExec(socket_t fd) {
    return ::evutil_make_socket_closeonexec(fd);
}

void setTimeout(socket_t fd, uint32_t timeout_ms) {
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    int ret = ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&tv, sizeof(tv));
    runtimeAssert(ret == 0);
    if (ret != 0) {
        LOG_ERROR("setsockopt SO_RCVTIMEO ERROR, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
    }
}

void setTimeout(socket_t fd, const Duration &timeout) {
    setTimeout(fd, (uint32_t)(timeout.milliseconds()));
}

void setKeepAlive(socket_t fd, bool on, int seconds) {
    int optval = on ? 1 : 0;
    int rc = ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const void *)&optval, sizeof(optval));
    if (rc != 0) {
        LOG_ERROR("setsockopt(SO_KEEPALIVE) failed, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
        return;
    }
    if (on && seconds > 0) {
#ifdef __linux__
        // Default settings are more or less garbage, with the keepalive time
        // set to 7200 by default on Linux. Modify settings to make the feature actually useful.

        // Send first probe after interval.
        optval = seconds;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &optval, sizeof(optval)) < 0) {
            LOG_ERROR("setsockopt(TCP_KEEPIDLE) failed, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
            return;
        }

        // Send next probes after the specified interval. Note that we set the
        // delay as interval / 3, as we send three probes before detecting
        // an error (see the next setsockopt call).
        optval = seconds / 3;
        if (optval == 0) optval = 1;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &optval, sizeof(optval)) < 0) {
            LOG_ERROR("setsockopt(TCP_KEEPINTVL) failed, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
            return;
        }

        // Consider the socket in error state after three we send three ACK
        // probes without getting a reply.
        optval = 3;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &optval, sizeof(optval)) < 0) {
            LOG_ERROR("setsockopt(TCP_KEEPCNT) failed, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
            return;
        }
#elif defined(__APPLE__)
        // Set idle time with interval
        optval = seconds;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &optval, sizeof(optval)) < 0) {
            LOG_ERROR("setsockopt(TCP_KEEPALIVE) failed, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
            return;
        }
#else
        ((void)seconds); // Avoid unused var warning for non Linux systems.
#endif
    }
}

void setReuseAddr(socket_t fd) {
    int optval = 1;
    int rc = ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(optval));
    if (rc != 0) {
        LOG_ERROR("setsockopt(SO_REUSEADDR) failed, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
    }
}

void setReusePort(socket_t fd) {
#ifdef SO_REUSEPORT
    int optval = 1;
    int rc = ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const void *)&optval, sizeof(optval));
    if (rc != 0) {
        LOG_ERROR("setsockopt(SO_REUSEPORT) failed, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
    }
#endif
}

void setTcpNoDelay(socket_t fd, bool on) {
    int optval = on ? 1 : 0;
    int rc = ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const void *)&optval, sizeof(optval));
    if (rc != 0) {
        LOG_ERROR("setsockopt(TCP_NODELAY) failed, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
    }
}

uint32_t ip2Number(const char *address) {
    struct in_addr addr;
    return ::inet_aton(address, &addr) != 0 ? addr.s_addr : -1;
}

bool parseFromIPPort(const char *address, struct sockaddr_storage &ss) {
    ::memset(&ss, 0, sizeof(ss));
    bool is_tls;
    std::string host;
    int port;
    if (!splitTriadicAddress(address, is_tls, host, port)) {
        return false;
    }
    short family = AF_INET;
    auto index = host.find(':');
    if (index != std::string::npos) {
        family = AF_INET6;
    }
    struct sockaddr_in *addr = sockaddr_in_cast(&ss);
    int rc = ::evutil_inet_pton(family, host.data(), &addr->sin_addr);
    if (rc == 0) {
        LOG_INFO(
            "ParseFromIPPort evutil_inet_pton (AF_INET '{}', ...) rc=0. "
            "{} is not a valid IP address. Maybe it is a hostname",
            host.data(), host.data());
        return false;
    } else if (rc < 0) {
        if (errno == 0) {
            LOG_INFO("[{}] is not a IP address. Maybe it is a hostname", host.data());
        } else {
            LOG_WARN("ParseFromIPPort evutil_inet_pton (AF_INET, '{}', ...) failed, error: {} -> {}",
                     host.data(), errno, SystemUtil::errnoToString(errno));
        }
        return false;
    }
    addr->sin_family = family;
    addr->sin_port = htons(port);

    return true;
}

bool splitTriadicAddress(const char *address, bool &is_tls, std::string &host, int &port) {
    std::string a = address;
    if (a.empty() or a.length() <= 6) {
        LOG_ERROR("Triadic Address is too short. addr: {}", a);
        return false;
    }

    is_tls = false;
    if (!a.compare(0, 6, "tls://")) {
        is_tls = true;
        a = a.substr(6);
    } else if (!a.compare(0, 6, "tcp://")) {
        is_tls = false;
        a = a.substr(6);
    }

    size_t index = a.rfind(':');
    if (index == std::string::npos) {
        LOG_ERROR("Address specified error <{}>. Cannot find ':'", address);
        return false;
    }
    if (index == a.size() - 1) {
        return false;
    }
    port = std::atoi(&a[index + 1]);
    host = std::string(a, 0, index);
    if (host[0] == '[') {
        if (*host.rbegin() != ']') {
            LOG_ERROR(
                "Address specified error <"
                ">. '[' ']' is not pair",
                address);
            return false;
        }
        // trim the leading '[' and trail ']'
        host = std::string(host.data() + 1, host.size() - 2);
    }
    // Compatible with "fe80::886a:49f3:20f3:add2]:80"
    if (*host.rbegin() == ']') {
        // trim the trail ']'
        host = std::string(host.data(), host.size() - 1);
    }

    return true;
}

struct sockaddr_storage getLocalAddr(socket_t sockfd) {
    struct sockaddr_storage laddr;
    memset(&laddr, 0, sizeof laddr);
    socklen_t addrlen = sizeof(laddr);
    if (::getsockname(sockfd, sockaddr_cast(&laddr), &addrlen) < 0) {
        LOG_ERROR("getLocalAddr failed, errno: {} -> {}", errno, SystemUtil::errnoToString(errno));
        memset(&laddr, 0, sizeof laddr);
    }
    return laddr;
}

std::string toIPPort(const struct sockaddr_storage *ss) {
    std::string saddr;
    int port = 0;
    if (ss->ss_family == AF_INET) {
        const struct sockaddr_in *addr4 = (const struct sockaddr_in *)sockaddr_in_cast(ss);
        char buf[INET_ADDRSTRLEN] = {};
        const char *addr = ::evutil_inet_ntop(ss->ss_family, &addr4->sin_addr, buf, INET_ADDRSTRLEN);
        if (addr) {
            saddr = addr;
        }
        port = ntohs(addr4->sin_port);
    } else if (ss->ss_family == AF_INET6) {
        const struct sockaddr_in6 *addr6 = (const struct sockaddr_in6 *)sockaddr_in6_cast(ss);
        char buf[INET6_ADDRSTRLEN] = {};
        const char *addr = ::evutil_inet_ntop(ss->ss_family, &addr6->sin6_addr, buf, INET6_ADDRSTRLEN);
        if (addr) {
            saddr = std::string("[") + addr + "]";
        }
        port = ntohs(addr6->sin6_port);
    } else {
        LOG_ERROR("unknown socket family connected");
        return "";
    }
    if (!saddr.empty()) {
        saddr.append(":", 1).append(std::to_string(port));
    }
    return saddr;
}

std::string toIPPort(const struct sockaddr *ss) {
    return toIPPort(sockaddr_storage_cast(ss));
}

std::string toIPPort(const struct sockaddr_in *ss) {
    return toIPPort(sockaddr_storage_cast(ss));
}

std::string toIP(const struct sockaddr *s) {
    auto ss = sockaddr_storage_cast(s);
    if (ss->ss_family == AF_INET) {
        const struct sockaddr_in *addr4 = (const struct sockaddr_in *)sockaddr_in_cast(ss);
        char buf[INET_ADDRSTRLEN] = {};
        const char *addr = ::evutil_inet_ntop(ss->ss_family, &addr4->sin_addr, buf, INET_ADDRSTRLEN);
        if (addr) {
            return std::string(addr);
        }
    } else if (ss->ss_family == AF_INET6) {
        const struct sockaddr_in6 *addr6 = (const struct sockaddr_in6 *)sockaddr_in6_cast(ss);
        char buf[INET6_ADDRSTRLEN] = {};
        const char *addr = ::evutil_inet_ntop(ss->ss_family, &addr6->sin6_addr, buf, INET6_ADDRSTRLEN);
        if (addr) {
            return std::string(addr);
        }
    } else {
        LOG_ERROR("unknown socket family connected");
    }
    return "";
}

std::string toIP(const struct sockaddr_in *ss) {
    return toIP(sockaddr_cast(ss));
}

bool getLocalAddrs(std::vector<std::string> &localaddrs) {
    struct ifaddrs *if_addrs = nullptr;

    ::getifaddrs(&if_addrs);
    if (!if_addrs) {
        return false;
    }
    for (auto ifa = if_addrs; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) {
            auto addr_in = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addr_str[INET_ADDRSTRLEN];
            ::inet_ntop(AF_INET, addr_in, addr_str, INET_ADDRSTRLEN);
            localaddrs.emplace_back(addr_str);
        } else if (ifa->ifa_addr->sa_family == AF_INET6) {
            auto addr_in = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addr_str[INET6_ADDRSTRLEN];
            ::inet_ntop(AF_INET6, addr_in, addr_str, INET6_ADDRSTRLEN);
            localaddrs.emplace_back(addr_str);
        }
    }
    ::freeifaddrs(if_addrs);
    return true;
}

} // namespace sockets
} // namespace tair::network
