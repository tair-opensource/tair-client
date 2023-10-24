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

#include <algorithm>

#include "protocol/packet/resp/ArrayPacket.hpp"
#include "client/TairTransactionClient.hpp"
#include "TairClient_Standalone_Server.hpp"

using tair::protocol::PacketPtr;
using tair::protocol::ArrayPacket;
using tair::client::TairTransactionClient;
using tair::common::CountDownLatch;

TEST_F(StandAloneTest, MULTI) {
    auto client = std::make_unique<TairTransactionClient>();
    client->setServerAddr(STANDALONE_ADDR);
    ASSERT_TRUE(client->connect().get().isSuccess());
    ASSERT_TRUE(client->isConnected());

    client->multi();
    client->appendCommand({"set", "key", "value"});
    client->appendCommand({"get", "key"});

    CountDownLatch latch;
    client->exec([&](auto *, auto &, const PacketPtr &resp) {
        resp->packet_cast<ArrayPacket>()->getPacketArray();
        latch.countDown();
    });
    latch.wait();
}