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

#include "network/EventLoop.hpp"
#include "network/EventLoopThread.hpp"
#include "network/TcpClient.hpp"
#include "network/TcpConnection.hpp"
#include "network/TcpServer.hpp"

using tair::network::Duration;
using tair::network::EventLoop;
using tair::network::EventLoopThread;
using tair::network::TcpServer;
using tair::network::TcpClient;
using tair::network::TcpClientPtr;
using tair::network::TcpConnectionPtr;
using tair::network::Buffer;

TEST(SERVER_CLIENT_TEST, CONN_TEST) {
    EventLoop loop;
    TcpServer server(&loop, "tcp://127.0.0.1:0", 2, "echo");
    std::string address;
    server.setConnectionCallback([&address](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
        if (conn->isConnected()) {
            ASSERT_EQ(address, conn->getLocalIpPort());
            ASSERT_NE(0, conn->sockfd());
        }
    });
    server.setClosedCallback([&]() {
        loop.stop();
    });

    ASSERT_TRUE(server.start());
    address = *server.getRealListenIpPorts().begin();

    std::atomic_int connected_count = 0;
    std::vector<TcpClientPtr> clients;
    for (size_t i = 0; i < 10; ++i) {
        auto client = TcpClient::create(&loop, address);
        client->setConnectingTimeout(Duration(1 * Duration::kSecond));
        client->setConnectionCallback([&](const TcpConnectionPtr &conn) {
            ASSERT_TRUE(conn->loop()->isInLoopThread());
            if (conn->isConnected()) {
                connected_count++;
            }
        });
        client->connect();
        if (i % 2 == 0) {
            client->disconnect();
        }
        clients.push_back(std::move(client));
    }

    loop.runAfterTimer(Duration(2 * Duration::kSecond), [&](EventLoop *) {
        for (auto &client : clients) {
            client->disconnect();
        }
        server.stop();
    });

    loop.run();
    ASSERT_EQ(5, connected_count);
}

TEST(SERVER_CLIENT_TEST, OTHER_IO_CONN_TEST) {
    EventLoopThread loop_thread;
    loop_thread.start();
    EventLoop *loop = loop_thread.loop();
    ASSERT_TRUE(loop != nullptr);

    TcpServer server(loop, "tcp://127.0.0.1:0", 2, "echo");
    std::string address;
    server.setConnectionCallback([&address](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
        if (conn->isConnected()) {
            ASSERT_EQ(address, conn->getLocalIpPort());
            ASSERT_NE(0, conn->sockfd());
        }
    });
    server.setClosedCallback([&]() {
        loop_thread.stop();
    });

    ASSERT_TRUE(server.start());
    address = *server.getRealListenIpPorts().begin();

    std::atomic_int connected_count = 0;
    std::vector<TcpClientPtr> clients;
    for (size_t i = 0; i < 10; ++i) {
        auto client = TcpClient::create(loop, address);
        client->setConnectingTimeout(Duration(1 * Duration::kSecond));
        client->setConnectionCallback([address, &connected_count](const TcpConnectionPtr &conn) {
            if (conn->isConnected()) {
                ASSERT_TRUE(conn->loop()->isInLoopThread());
                ASSERT_TRUE(conn->isConnected());
                ASSERT_EQ(address, conn->getRemoteIpPort());
                ASSERT_NE(0, conn->sockfd());
                connected_count += 1;
            }
        });
        client->connect();
        if (i % 2 == 0) {
            client->disconnect();
        }
        clients.push_back(std::move(client));
    }

    loop->runAfterTimer(Duration(1 * Duration::kSecond), [&](EventLoop *) {
        for (auto &client : clients) {
            client->disconnect();
        }
        server.stop();
    });

    loop_thread.join();
    ASSERT_EQ(5, connected_count);
}

TEST(SERVER_CLIENT_TEST, SEND_TEST) {
    EventLoop loop;
    TcpServer server(&loop, "tcp://127.0.0.1:0", 2, "echo");
    std::string address;
    server.setConnectionCallback([&address](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
        if (conn->isConnected()) {
            ASSERT_EQ(address, conn->getLocalIpPort());
            ASSERT_NE(0, conn->sockfd());
        }
    });
    server.setMessageCallback([](const TcpConnectionPtr &conn, Buffer *buf) {
        auto data = buf->nextAllString();
        ASSERT_EQ("hello", data);
        conn->send(data);
    });
    server.setWriteCompleteCallback([](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
    });
    server.setClosedCallback([&]() {
        loop.stop();
    });

    ASSERT_TRUE(server.start());
    address = *server.getRealListenIpPorts().begin();

    auto client = TcpClient::create(&loop, address);
    client->setConnectingTimeout(Duration(1 * Duration::kSecond));
    client->setConnectionCallback([address](const TcpConnectionPtr &conn) {
        if (conn->isConnected()) {
            ASSERT_TRUE(conn->loop()->isInLoopThread());
            ASSERT_TRUE(conn->isConnected());
            ASSERT_EQ(address, conn->getRemoteIpPort());
            ASSERT_NE(0, conn->sockfd());
            conn->send("hello");
        }
    });
    client->setMessageCallback([](const TcpConnectionPtr &conn, Buffer *buf) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
        ASSERT_EQ("hello", buf->nextAllString());
    });
    client->setWriteCompleteCallback([](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
    });
    client->connect();

    loop.runAfterTimer(Duration(1 * Duration::kSecond), [&](EventLoop *) {
        client->disconnect();
        server.stop();
    });

    loop.run();
}

TEST(SERVER_CLIENT_TEST, OTHER_IO_SEND_STRING_MOVE_TEST) {
    EventLoopThread loop_thread;
    loop_thread.start();
    EventLoop *loop = loop_thread.loop();
    ASSERT_TRUE(loop != nullptr);

    TcpServer server(loop, "tcp://127.0.0.1:0", 2, "echo");
    std::string address;
    server.setConnectionCallback([&address](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
        if (conn->isConnected()) {
            ASSERT_EQ(address, conn->getLocalIpPort());
            ASSERT_NE(0, conn->sockfd());
        }
    });
    server.setMessageCallback([](const TcpConnectionPtr &conn, Buffer *buffer) {
        auto data = buffer->nextAllString();
        ASSERT_EQ("hello", data);
        conn->send(data);
    });
    server.setWriteCompleteCallback([](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
    });
    server.setClosedCallback([&]() {
        loop_thread.stop();
    });

    ASSERT_TRUE(server.start());
    address = *server.getRealListenIpPorts().begin();

    std::atomic_int connected_count = 0;
    auto client = TcpClient::create(loop, address);
    client->setConnectingTimeout(Duration(1 * Duration::kSecond));
    client->setConnectionCallback([address, &connected_count](const TcpConnectionPtr &conn) {
        if (conn->isConnected()) {
            connected_count += 1;
            ASSERT_TRUE(conn->loop()->isInLoopThread());
            ASSERT_TRUE(conn->isConnected());
            ASSERT_EQ(address, conn->getRemoteIpPort());
            ASSERT_NE(0, conn->sockfd());
        }
    });
    client->setMessageCallback([](const TcpConnectionPtr &conn, Buffer *buffer) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
        ASSERT_EQ("hello", buffer->nextAllString());
    });
    client->setWriteCompleteCallback([](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
    });
    client->connect();

    loop->runAfterTimer(Duration(1 * Duration::kSecond), [&](EventLoop *) {
        client->disconnect();
        client.reset();
        server.stop();
    });

    while (!client->connection()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::string info = "hello";
    ASSERT_EQ(5, info.size());
    client->connection()->send(std::move(info));
    ASSERT_EQ(0, info.size());

    loop_thread.join();
    ASSERT_EQ(1, connected_count);
}

TEST(SERVER_CLIENT_TEST, OTHER_IO_SEND_BUFFER_MOVE_TEST) {
    EventLoopThread loop_thread;
    loop_thread.start();
    EventLoop *loop = loop_thread.loop();
    ASSERT_TRUE(loop != nullptr);

    TcpServer server(loop, "tcp://127.0.0.1:0", 2, "echo");
    std::string address;
    server.setConnectionCallback([&address](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
        if (conn->isConnected()) {
            ASSERT_EQ(address, conn->getLocalIpPort());
            ASSERT_NE(0, conn->sockfd());
        }
    });
    server.setMessageCallback([](const TcpConnectionPtr &conn, Buffer *buffer) {
        auto data = buffer->nextAllString();
        ASSERT_EQ("hello", data);
        conn->send(data);
    });
    server.setWriteCompleteCallback([](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
    });
    server.setClosedCallback([&]() {
        loop_thread.stop();
    });

    ASSERT_TRUE(server.start());
    address = *server.getRealListenIpPorts().begin();

    auto client = TcpClient::create(loop, address);
    client->setConnectingTimeout(Duration(1 * Duration::kSecond));
    client->setConnectionCallback([address](const TcpConnectionPtr &conn) {
        if (conn->isConnected()) {
            ASSERT_TRUE(conn->loop()->isInLoopThread());
            ASSERT_TRUE(conn->isConnected());
            ASSERT_EQ(address, conn->getRemoteIpPort());
            ASSERT_NE(0, conn->sockfd());
        }
    });
    client->setMessageCallback([](const TcpConnectionPtr &conn, Buffer *buffer) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
        ASSERT_EQ("hello", buffer->nextAllString());
    });
    client->setWriteCompleteCallback([](const TcpConnectionPtr &conn) {
        ASSERT_TRUE(conn->loop()->isInLoopThread());
    });
    client->connect();

    loop->runAfterTimer(Duration(1 * Duration::kSecond), [&](EventLoop *) {
        client->disconnect();
        client.reset();
        server.stop();
    });

    while (!client->connection()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    Buffer buf;
    buf.append("hello");
    ASSERT_EQ(5, buf.size());
    client->connection()->send(std::move(buf));
    ASSERT_EQ(0, buf.size());

    loop_thread.join();
}
