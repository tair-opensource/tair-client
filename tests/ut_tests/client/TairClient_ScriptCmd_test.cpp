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
#include "TairClient_Standalone_Server.hpp"

using tair::protocol::PacketPtr;
using tair::protocol::ArrayPacket;
using tair::protocol::BulkStringPacket;
using tair::common::CountDownLatch;

TEST_F(StandAloneTest, EVAL) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    auto packet = wrapper.eval("return 'hello'", {}, {}).get();
    ASSERT_EQ("hello", packet->packet_cast<BulkStringPacket>()->getValue());
}

TEST_F(StandAloneTest, SCRIPT_LOAD) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    auto sha1 = wrapper.scriptLoad("return KEYS[1]").get().getValue();
    auto packet = wrapper.evalsha(sha1, {"foo"}, {}).get();
    ASSERT_EQ("foo", packet->packet_cast<BulkStringPacket>()->getValue());
}

TEST_F(StandAloneTest, SCRIPT_FLUSH) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    auto sha1 = wrapper.scriptLoad("return KEYS[1]").get().getValue();
    auto ret = wrapper.scriptExists({sha1}).get().getValue();
    ASSERT_EQ(1, ret.size());
    ASSERT_EQ(1, ret[0]);
    ASSERT_EQ("OK", wrapper.scriptFlush().get().getValue());
}