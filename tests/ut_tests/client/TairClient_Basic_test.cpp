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

#include "common/CountDownLatch.hpp"
#include "common/Logger.hpp"
#include "protocol/packet/resp/ArrayPacket.hpp"
#include "protocol/packet/resp/ErrorPacket.hpp"
#include "protocol/packet/resp/VerbatimStringPacket.hpp"
#include "client/TairAsyncClient.hpp"
#include "client/TairBaseClient.hpp"
#include "client/TairClient.hpp"
#include "client/TairSubscribeClient.hpp"

#include "TairClient_Standalone_Server.hpp"

using tair::common::CountDownLatch;
using tair::client::TairBaseClient;
using tair::client::TairAsyncClient;
using tair::client::TairClientWrapper;
using tair::client::TairSubscribeClient;
using tair::client::PubSubCount;
using tair::client::SubMessage;
using tair::client::PSubMessage;
using tair::client::TairClient;
using tair::client::TairResult;
using tair::client::TairURI;
using tair::network::EventLoopThread;
using tair::network::TcpConnectionPtr;
using tair::network::Duration;
using tair::protocol::PacketPtr;
using tair::protocol::ErrorPacket;
using tair::protocol::SimpleStringPacket;
using tair::protocol::BulkStringPacket;
using tair::protocol::IntegerPacket;
using tair::protocol::VerbatimStringPacket;

TEST_F(StandAloneTest, CONN_FAILED_TEST) {
    CountDownLatch latch;
    auto client = std::make_unique<TairBaseClient>();
    client->setServerAddr("127.0.0.1:2001");
    ASSERT_TRUE(!client->connect().get().isSuccess());
}

TEST_F(StandAloneTest, ASYNC_SEND_COMMAND_TEST) {
    auto client = std::make_unique<TairAsyncClient>();
    client->setServerAddr(STANDALONE_ADDR);
    client->setAutoReconnect(false);
    ASSERT_TRUE(client->connect().get().isSuccess());
    ASSERT_TRUE(client->isConnected());

    client->sendCommand({"set", "key", "value"}, [](auto *, auto &, const PacketPtr &resp) {
        if (resp) {
            auto simple = resp->packet_cast<SimpleStringPacket>();
            ASSERT_TRUE(simple);
            LOG_INFO("set result: {}", simple->getValue());
        }
    });

    client->sendCommand({"get", "key"}, [](auto *, auto &, const PacketPtr &resp) {
        if (resp) {
            auto bulk = resp->packet_cast<BulkStringPacket>();
            ASSERT_TRUE(bulk);
            LOG_INFO("get result: {}", bulk->getValue());
        }
    });

    client->sendCommand({"del", "key"}, [](auto *, auto &, const PacketPtr &resp) {
        if (resp) {
            auto integer = resp->packet_cast<IntegerPacket>();
            ASSERT_TRUE(integer);
            LOG_INFO("del result: {}", integer->getValue());
        }
    });

    CountDownLatch latch;
    client->sendCommand({"quit"}, [&latch](auto *, auto &, auto &) {
        latch.countDown();
    });
    latch.wait();
}

TEST_F(StandAloneTest, ASYNC_API_TEST) {
    auto client = std::make_unique<TairAsyncClient>();
    client->setServerAddr(STANDALONE_ADDR);
    ASSERT_TRUE(client->connect().get().isSuccess());
    ASSERT_TRUE(client->isConnected());

    client->set("key", "value", [](auto &resp) {
        ASSERT_TRUE(resp.isSuccess());
        ASSERT_EQ("OK", resp.getValue());
        LOG_INFO("set result: {}", resp.getValue());
    });

    client->get("key", [](auto &resp) {
        ASSERT_TRUE(resp.isSuccess());
        ASSERT_EQ("value", *(resp.getValue()));
        LOG_INFO("get result: {}", *(resp.getValue()));
    });

    client->del("key", [](auto &resp) {
        ASSERT_TRUE(resp.isSuccess());
        ASSERT_EQ(1, resp.getValue());
        LOG_INFO("del result: {}", resp.getValue());
    });

    CountDownLatch latch;
    client->quit([&latch](auto &resp) {
        ASSERT_TRUE(resp.isSuccess());
        ASSERT_EQ("OK", resp.getValue());
        LOG_INFO("quit result: {}", resp.getValue());
        latch.countDown();
    });
    latch.wait();
}

TEST_F(StandAloneTest, SYNC_API_TEST) {
    auto wrapper = client->getFutureWrapper();

    auto result_set = wrapper.set("key", "value").get();
    ASSERT_TRUE(result_set.isSuccess());
    ASSERT_EQ("OK", result_set.getValue());

    auto result_get = wrapper.get("key").get();
    ASSERT_TRUE(result_get.isSuccess());
    ASSERT_EQ("value", *(result_get.getValue()));

    auto result_del = wrapper.del("key").get();
    ASSERT_TRUE(result_del.isSuccess());
    ASSERT_EQ(1, result_del.getValue());

    auto result_quit = wrapper.quit().get();
    ASSERT_TRUE(result_set.isSuccess());
    ASSERT_EQ("OK", result_set.getValue());
}

TEST_F(StandAloneTest, PUBSUB_TEST) {
    auto sub_client = std::make_unique<TairSubscribeClient>();
    sub_client->setServerAddr(STANDALONE_ADDR);
    ASSERT_TRUE(sub_client->connect().get().isSuccess());
    ASSERT_TRUE(sub_client->isConnected());

    auto client = std::make_unique<TairAsyncClient>();
    client->setServerAddr(STANDALONE_ADDR);
    ASSERT_TRUE(client->connect().get().isSuccess());
    ASSERT_TRUE(client->isConnected());

    CountDownLatch latch(4);

    std::string channel = "channel_test";
    std::string message = "message_test";

    auto callback = [&](auto &result) {
        ASSERT_TRUE(result.isSuccess());
        auto &value = result.getValue();
        ASSERT_EQ("subscribe", value.type);
        ASSERT_EQ(channel, value.name);
        ASSERT_EQ(1, value.count);

        client->publish(channel, message, [&](auto &result) {
            ASSERT_TRUE(result.isSuccess());
            ASSERT_EQ(1, result.getValue());
            latch.countDown();
        });

        client->publish(channel, message, [&](auto &result) {
            ASSERT_TRUE(result.isSuccess());
            ASSERT_EQ(1, result.getValue());
            latch.countDown();
        });
    };
    auto msg_callback = [&](auto &msg) {
        ASSERT_EQ(channel, msg.channel);
        ASSERT_EQ(message, msg.message);
        latch.countDown();
    };
    sub_client->subscribe(channel, callback, msg_callback);

    latch.wait();

    client->disconnect();
    sub_client->disconnect();
}

TEST_F(StandAloneTest, PUBPSUB_TEST) {
    auto sub_client = std::make_unique<TairSubscribeClient>();
    sub_client->setServerAddr(STANDALONE_ADDR);
    sub_client->setKeepAliveSeconds(10);
    ASSERT_TRUE(sub_client->connect().get().isSuccess());
    ASSERT_TRUE(sub_client->isConnected());

    auto client = std::make_unique<TairAsyncClient>();
    client->setServerAddr(STANDALONE_ADDR);
    ASSERT_TRUE(client->connect().get().isSuccess());
    ASSERT_TRUE(client->isConnected());

    CountDownLatch latch(4);

    std::string pattern = "channel_*";
    std::string channel = "channel_test";
    std::string message = "message_test";

    auto callback = [&](auto &result) {
        ASSERT_TRUE(result.isSuccess());
        auto &value = result.getValue();
        ASSERT_EQ("psubscribe", value.type);
        ASSERT_EQ(pattern, value.name);
        ASSERT_EQ(1, value.count);

        client->publish(channel, message, [&](auto &result) {
            ASSERT_TRUE(result.isSuccess());
            ASSERT_EQ(1, result.getValue());
            latch.countDown();
        });

        client->publish(channel, message, [&](auto &result) {
            ASSERT_TRUE(result.isSuccess());
            ASSERT_EQ(1, result.getValue());
            latch.countDown();
        });
    };
    auto pmsg_callback = [&](auto &msg) {
        ASSERT_EQ(pattern, msg.pattern);
        ASSERT_EQ(channel, msg.channel);
        ASSERT_EQ(message, msg.message);
        latch.countDown();
    };
    sub_client->psubscribe(pattern, callback, pmsg_callback);
    latch.wait();
}

TEST_F(StandAloneTest, TAIRCLIENT_SETGET) {
    client->set("key", "value", [](const TairResult<std::string> &result) {
        ASSERT_TRUE(result.isSuccess());
        ASSERT_EQ("OK", result.getValue());
    });

    CountDownLatch latch;
    client->get("key", [&latch](const TairResult<std::shared_ptr<std::string>> &result) {
        ASSERT_TRUE(result.isSuccess());
        ASSERT_EQ("value", *(result.getValue()));
        latch.countDown();
    });
    latch.wait();
}

TEST_F(StandAloneTest, SETNAME) {
    client->sendCommand({"client", "list"}, [](auto *, auto &, const PacketPtr &resp) {
        if (resp) {
            auto simple = resp->packet_cast<BulkStringPacket>();
            ASSERT_TRUE(simple);
            const auto &result = simple->getValue();
            ASSERT_TRUE(result.find("version") != std::string::npos);
        }
    });

    CountDownLatch latch;
    client->sendCommand({"quit"}, [&latch](auto *, auto &, auto &) {
        latch.countDown();
    });
    latch.wait();
}

TEST_F(StandAloneTest, RECONNECT) {
    auto client = std::make_unique<TairAsyncClient>();
    client->setServerAddr(STANDALONE_ADDR);
    client->setConnectingTimeoutMs(10000);
    client->setReconnectIntervalMs(2000);
    ASSERT_TRUE(client->connect().get().isSuccess());
    ASSERT_TRUE(client->isConnected());

    CountDownLatch l1;
    client->set("key", "value", [&l1](const TairResult<std::string> &result) {
        ASSERT_TRUE(result.isSuccess());
        ASSERT_EQ("OK", result.getValue());
        l1.countDown();
    });
    l1.wait();

    CountDownLatch l2;
    client->sendCommand({"debug", "sleep", "5"}, [&l2](auto *, auto &, const PacketPtr &resp) {
        ASSERT_TRUE(resp == nullptr);
        l2.countDown();
    });
    l2.wait();
    int count = 0;
    while (!client->isConnected() && ++count < 2000) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    ASSERT_TRUE(client->isConnected());

    CountDownLatch l3;
    client->get("key", [&l3](const TairResult<std::shared_ptr<std::string>> &result) {
        if (result.getValue()) {
            ASSERT_TRUE(result.isSuccess());
            ASSERT_EQ("value", *result.getValue());
        }
        l3.countDown();
    });
    l3.wait();

    client->destroy();
}
