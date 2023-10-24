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

#include <cstring>
#include <string>
#include <vector>

#include "network/Duration.hpp"
#include "network/Event.hpp"
#include "network/Types.hpp"

namespace tair::network {
namespace sockets {

int createSocketPair(int domain, int type, int protocol, socket_t sock[2]);
socket_t createNonblockingSocket();
int connectSocket(socket_t fd, const struct sockaddr *addr, socklen_t len);
int bindSocket(socket_t fd, const struct sockaddr *addr, socklen_t addrlen);
int listenSocket(socket_t fd, int backlog = SOMAXCONN);
int acceptSocket(socket_t fd, struct sockaddr *addr, socklen_t *addrlen);
void closeSocket(socket_t fd);
int readVFromSocket(socket_t fd, const struct iovec *vec, int iovcnt);
int readFromSocket(socket_t fd, void *buf, int len);
int writeToSocket(socket_t fd, const void *buf, size_t len);

int getSocketErrorCode(socket_t fd);

void setKeepAlive(socket_t fd, bool on, int seconds);
void setReuseAddr(socket_t fd);
void setReusePort(socket_t fd);
void setTcpNoDelay(socket_t fd, bool on);
int setNonBlocking(socket_t fd);
int setCloseOnExec(socket_t fd);
void setTimeout(socket_t fd, uint32_t timeout_ms);
void setTimeout(socket_t fd, const Duration &timeout);

std::string toIPPort(const struct sockaddr_storage *ss);
std::string toIPPort(const struct sockaddr *ss);
std::string toIPPort(const struct sockaddr_in *ss);
std::string toIP(const struct sockaddr *ss);
std::string toIP(const struct sockaddr_in *ss);

bool getVirtualIPPort(socket_t sockfd, std::string &ip, int &port, uint32_t &vid);
bool getLocalAddrs(std::vector<std::string> &localaddrs);

inline bool isZeroAddress(const struct sockaddr_storage *ss) {
    const char *p = reinterpret_cast<const char *>(ss);
    for (size_t i = 0; i < sizeof(*ss); ++i) {
        if (p[i] != 0) {
            return false;
        }
    }
    return true;
}

uint32_t ip2Number(const char *address);

// @brief Parse a literal network address and return an internet protocol family address
// @param[in] address - A network address of the form "host:port" or "[host]:port"
// @return bool - false if parse failed.
bool parseFromIPPort(const char *address, struct sockaddr_storage &ss);

inline struct sockaddr_storage parseFromIPPort(const char *address) {
    struct sockaddr_storage ss;
    bool rc = parseFromIPPort(address, ss);
    if (rc) {
        return ss;
    } else {
        ::memset(&ss, 0, sizeof(ss));
        return ss;
    }
}

// @brief Splits a triadic network address of the form "tcp://host:port", "tcp://[host]:port" or start by "tls://"
//  into host and port. A literal address or host name for IPv6
// must be enclosed in square brackets, as in "[::1]:80" or "[ipv6-host]:80"
// @param[in] address - A triadic network address
// @param[out] is_tls -
// @param[out] host -
// @param[out] port - the port in local machine byte order
// @return bool - false if the network address is invalid format
bool splitTriadicAddress(const char *address, bool &is_tls, std::string &host, int &port);
struct sockaddr_storage getLocalAddr(socket_t sockfd);

template <typename To, typename From>
inline To implicit_cast(From const &f) {
    return f;
}

inline const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr) {
    return static_cast<const struct sockaddr *>(sockets::implicit_cast<const void *>(addr));
}

inline struct sockaddr *sockaddr_cast(struct sockaddr_in *addr) {
    return static_cast<struct sockaddr *>(sockets::implicit_cast<void *>(addr));
}

inline struct sockaddr *sockaddr_cast(struct sockaddr_storage *addr) {
    return static_cast<struct sockaddr *>(sockets::implicit_cast<void *>(addr));
}

inline const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr) {
    return static_cast<const struct sockaddr_in *>(sockets::implicit_cast<const void *>(addr));
}

inline struct sockaddr_in *sockaddr_in_cast(struct sockaddr *addr) {
    return static_cast<struct sockaddr_in *>(sockets::implicit_cast<void *>(addr));
}

inline struct sockaddr_in *sockaddr_in_cast(struct sockaddr_storage *addr) {
    return static_cast<struct sockaddr_in *>(sockets::implicit_cast<void *>(addr));
}

inline struct sockaddr_in6 *sockaddr_in6_cast(struct sockaddr_storage *addr) {
    return static_cast<struct sockaddr_in6 *>(sockets::implicit_cast<void *>(addr));
}

inline const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr_storage *addr) {
    return static_cast<const struct sockaddr_in *>(sockets::implicit_cast<const void *>(addr));
}

inline const struct sockaddr_in6 *sockaddr_in6_cast(const struct sockaddr_storage *addr) {
    return static_cast<const struct sockaddr_in6 *>(sockets::implicit_cast<const void *>(addr));
}

inline const struct sockaddr_storage *sockaddr_storage_cast(const struct sockaddr *addr) {
    return static_cast<const struct sockaddr_storage *>(sockets::implicit_cast<const void *>(addr));
}

inline const struct sockaddr_storage *sockaddr_storage_cast(const struct sockaddr_in *addr) {
    return static_cast<const struct sockaddr_storage *>(sockets::implicit_cast<const void *>(addr));
}

inline const struct sockaddr_storage *sockaddr_storage_cast(const struct sockaddr_in6 *addr) {
    return static_cast<const struct sockaddr_storage *>(sockets::implicit_cast<const void *>(addr));
}

} // namespace sockets
} // namespace tair::network
