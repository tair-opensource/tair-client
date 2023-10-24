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

#include "network/DnsResolver.hpp"
#include "network/EventLoopThread.hpp"

using tair::network::EventLoop;
using tair::network::EventLoopThread;
using tair::network::DnsResolver;
using tair::network::Duration;

TEST(DNS_RESOLVER_TEST, ONLY_TEST) {
    EventLoopThread loop_thread;
    loop_thread.start();

    ASSERT_TRUE(loop_thread.loop() != nullptr);
    auto resolver = DnsResolver::createDnsResolver(loop_thread.loop(),
                                                   "localhost", Duration(100 * Duration::kMillisecond),
                                                   [](int errcode, const std::vector<std::string> &addrs) {
                                                       ASSERT_EQ(0, errcode);
                                                       ASSERT_EQ(1U, addrs.size());
                                                       ASSERT_EQ("127.0.0.1", addrs[0]);
                                                   });
    resolver->start();
    resolver = DnsResolver::createDnsResolver(loop_thread.loop(),
                                              "error.addr", Duration(100 * Duration::kMillisecond),
                                              [](int errcode, const std::vector<std::string> &addrs) {
                                                  ASSERT_NE(0, errcode);
                                                  ASSERT_EQ(0U, addrs.size());
                                              });
    resolver->start();

    loop_thread.loop()->runAfterTimer(Duration(200 * Duration::kMillisecond), [&](EventLoop *) {
        loop_thread.stop();
    });
    loop_thread.join();
}
