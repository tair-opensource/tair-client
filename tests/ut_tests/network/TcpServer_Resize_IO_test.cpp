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
#include "network/EventLoop.hpp"
#include "network/EventLoopThread.hpp"
#include "network/TcpClient.hpp"
#include "network/TcpConnection.hpp"
#include "network/TcpServer.hpp"

#include "gtest/gtest.h"
#include <unistd.h>

using tair::network::Duration;
using tair::network::EventLoop;
using tair::network::EventLoopThread;
using tair::network::TcpServer;
using tair::network::TcpClient;
using tair::network::TcpClientPtr;
using tair::network::TcpConnectionPtr;
using tair::network::Buffer;

TEST(SERVER_RESIZE_IO_TEST, SERVER_RESIZE_IO_TEST) {
    EventLoopThread loop_thread;
    loop_thread.start();
    EventLoop *loop = loop_thread.loop();
    ASSERT_TRUE(loop != nullptr);

    TcpServer server(loop, "tcp://127.0.0.1:0", 2, "test");
    std::string address;
    server.setConnectionCallback([&address](const TcpConnectionPtr &conn) {
        int conn_count = std::any_cast<int>(conn->loop()->getContext());
        ASSERT_TRUE(conn->loop()->isInLoopThread());
        if (conn->isConnected()) {
            ASSERT_EQ(address, conn->getLocalIpPort());
            ASSERT_NE(0, conn->sockfd());
            conn_count++;
        } else {
            conn_count--;
        }
        conn->loop()->setContext(std::any(conn_count));
    });
    server.setLoopInitCallback([&](EventLoop *loop, int idx) {
        int conn_count = 0;
        loop->setContext(std::any(conn_count));
    });
    server.setAfterResizeThreadExitCheckCallBack([](size_t, EventLoop *loop) -> bool {
        int conn_count = std::any_cast<int>(loop->getContext());
        return conn_count == 0 && !loop->hasPendingTask();
    });
    server.setClosedCallback([&]() {
      loop_thread.stop();
    });

    ASSERT_TRUE(server.start());
    address = *server.getRealListenIpPorts().begin();

    ASSERT_EQ(2, server.ioThreadNum());
    ASSERT_EQ(2, server.availableIOThreadNum());

    server.resizeIOThreadPoolSize(4);

    ASSERT_EQ(4, server.ioThreadNum());
    ASSERT_EQ(4, server.availableIOThreadNum());

    server.resizeIOThreadPoolSize(2);

    ::sleep(2);

    ASSERT_EQ(2, server.ioThreadNum());
    ASSERT_EQ(2, server.availableIOThreadNum());

    server.resizeIOThreadPoolSize(4);

    ASSERT_EQ(4, server.ioThreadNum());
    ASSERT_EQ(4, server.availableIOThreadNum());

    std::deque<TcpClientPtr> clients;
    for (size_t i = 0; i < 100; ++i) {
        auto client = TcpClient::create(loop, address);
        client->setConnectingTimeout(Duration(1 * Duration::kSecond));
        client->setConnectionCallback([&](const TcpConnectionPtr &conn) {
            ASSERT_TRUE(conn->loop()->isInLoopThread());
        });
        client->connect();
        clients.push_back(std::move(client));
    }

    ::sleep(2);

    server.resizeIOThreadPoolSize(2);

    ASSERT_EQ(4, server.ioThreadNum());
    ASSERT_EQ(2, server.availableIOThreadNum());

    for (auto &client : clients) {
        client->disconnect();
        client.reset();
    }

    ::sleep(2);

    ASSERT_EQ(2, server.ioThreadNum());
    ASSERT_EQ(2, server.availableIOThreadNum());

    server.stop();
    loop_thread.join();
}
