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
#include "gtest/gtest.h"

#include "network/Sockets.hpp"

TEST(SOCKETS_TEST, ONLY_TEST) {
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = 16777343;
    sockaddr.sin_port = htons(6379);
    std::string ip = tair::network::sockets::toIP(&sockaddr);
    std::string ip_port = tair::network::sockets::toIPPort(&sockaddr);
    ASSERT_EQ("127.0.0.1", ip);
    ASSERT_EQ("127.0.0.1:6379", ip_port);

    std::vector<std::string> localaddrs;
    tair::network::sockets::getLocalAddrs(localaddrs);
    ASSERT_GT(localaddrs.size(), 0);
    for (const auto &addr : localaddrs) {
        fprintf(stderr, "%s\n", addr.c_str());
    }
}
