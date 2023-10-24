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

#include "network/Buffer.hpp"
#include "protocol/packet/resp/PushPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::IntegerPacket;
using tair::protocol::SimpleStringPacket;
using tair::protocol::DoublePacket;
using tair::protocol::PushPacket;

TEST(PUSH_PACKET_TEST, ENCODE_TEST) {
    PushPacket push1;
    Buffer buf;
    push1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), push1.getRESP3EncodeSize());
    ASSERT_EQ(">0\r\n", buf.nextAllString());
    buf.clear();
    push1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), push1.getRESP2EncodeSize());
    ASSERT_EQ("*0\r\n", buf.nextAllString());

    PushPacket push2;
    buf.clear();
    push2.addReplyInteger(10);
    push2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), push2.getRESP3EncodeSize());
    ASSERT_EQ(">1\r\n:10\r\n", buf.nextAllString());
    buf.clear();
    push2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), push2.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n:10\r\n", buf.nextAllString());

    PushPacket push3;
    buf.clear();
    push3.addReplyStatus("status");
    push3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), push3.getRESP3EncodeSize());
    ASSERT_EQ(">1\r\n+status\r\n", buf.nextAllString());
    buf.clear();
    push3.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), push3.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n+status\r\n", buf.nextAllString());

    PushPacket push4;
    buf.clear();
    push4.addReplyBulk("bulk");
    push4.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), push4.getRESP3EncodeSize());
    ASSERT_EQ(">1\r\n$4\r\nbulk\r\n", buf.nextAllString());
    buf.clear();
    push4.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), push4.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n$4\r\nbulk\r\n", buf.nextAllString());

    std::vector<int64_t> argv {1, 2, 3};
    PushPacket push5(argv);
    buf.clear();
    push5.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), push5.getRESP3EncodeSize());
    ASSERT_EQ(">3\r\n:1\r\n:2\r\n:3\r\n", buf.nextAllString());
    buf.clear();
    push5.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), push5.getRESP2EncodeSize());
    ASSERT_EQ("*3\r\n:1\r\n:2\r\n:3\r\n", buf.nextAllString());
    argv.clear();
    push5.moveIntegers(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ(1, argv[0]);
    ASSERT_EQ(2, argv[1]);
    ASSERT_EQ(3, argv[2]);

    std::vector<std::string> argv2 {"set", "key", "value"};
    PushPacket push6(argv2);
    buf.clear();
    push6.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), push6.getRESP3EncodeSize());
    ASSERT_EQ(">3\r\n$3\r\nset\r\n$3\r\nkey\r\n$5\r\nvalue\r\n", buf.nextAllString());
    buf.clear();
    push6.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), push6.getRESP2EncodeSize());
    ASSERT_EQ("*3\r\n$3\r\nset\r\n$3\r\nkey\r\n$5\r\nvalue\r\n", buf.nextAllString());
    argv2.clear();
    push6.moveBulks(argv2);
    ASSERT_EQ(3U, argv2.size());
    ASSERT_EQ(3U, argv2[0].size());
    ASSERT_EQ(3U, argv2[1].size());
    ASSERT_EQ(5U, argv2[2].size());

    PushPacket push7;
    push7.addReplyDouble(1.2);
    buf.clear();
    push7.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), push7.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n$3\r\n1.2\r\n", buf.nextAllString());
    buf.clear();
    push7.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), push7.getRESP3EncodeSize());
    ASSERT_EQ(">1\r\n,1.2\r\n", buf.nextAllString());
}

TEST(PUSH_PACKET_TEST, DECODE_TEST) {
    PushPacket push1;
    Buffer buf;
    ASSERT_EQ(DState::ERROR, push1.decodeRESP2(&buf));
    ASSERT_EQ(DState::AGAIN, push1.decodeRESP3(&buf));
    buf.append(":333\r\n");
    ASSERT_EQ(DState::ERROR, push1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '>', got ':'", push1.getDecodeErr());

    PushPacket push2;
    buf.clear();
    buf.append(">");
    buf.append(std::string(1024 * 65, 'a'));
    ASSERT_EQ(DState::ERROR, push2.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: too big count string", push2.getDecodeErr());

    PushPacket push3;
    buf.clear();
    buf.append(">\r\n");
    ASSERT_EQ(DState::ERROR, push3.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found array size", push3.getDecodeErr());

    PushPacket push4;
    buf.clear();
    buf.append(">123abc\r\n");
    ASSERT_EQ(DState::ERROR, push4.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: integer format error", push4.getDecodeErr());

    PushPacket push5;
    buf.clear();
    buf.append(">-2\r\n");
    ASSERT_EQ(DState::ERROR, push5.decodeRESP3(&buf));

    PushPacket push6;
    buf.clear();
    buf.append(">-1\r\n");
    ASSERT_EQ(DState::ERROR, push6.decodeRESP3(&buf));

    PushPacket push7;
    buf.clear();
    buf.append(">1\r\n");
    ASSERT_EQ(DState::AGAIN, push7.decodeRESP3(&buf));
    buf.append("-test\r\n");
    ASSERT_EQ(DState::SUCCESS, push7.decodeRESP3(&buf));
    ASSERT_EQ(11, push7.getRESP2EncodeSize());

    PushPacket push8;
    buf.clear();
    buf.append(">1\r\n");
    ASSERT_EQ(DState::AGAIN, push8.decodeRESP3(&buf));
    buf.append("&test\r\n");
    ASSERT_EQ(DState::ERROR, push8.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: unknown packet type: '&'", push8.getDecodeErr());

    PushPacket push9;
    buf.clear();
    buf.append(">1\r\n");
    ASSERT_EQ(DState::AGAIN, push9.decodeRESP3(&buf));
    buf.append(":\r\n");
    ASSERT_EQ(DState::ERROR, push9.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found integer", push9.getDecodeErr());

    PushPacket push10;
    buf.clear();
    buf.append(">");
    ASSERT_EQ(DState::AGAIN, push10.decodeRESP3(&buf));
    buf.append("2\r\n");
    ASSERT_EQ(DState::AGAIN, push10.decodeRESP3(&buf));
    buf.append(":");
    ASSERT_EQ(DState::AGAIN, push10.decodeRESP3(&buf));
    buf.append("10\r\n");
    ASSERT_EQ(DState::AGAIN, push10.decodeRESP3(&buf));
    buf.append("+");
    ASSERT_EQ(DState::AGAIN, push10.decodeRESP3(&buf));
    buf.append("OK\r\n");
    ASSERT_EQ(DState::SUCCESS, push10.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(14, push10.getRESP2EncodeSize());
    auto packets = push10.getPacketArray();
    ASSERT_EQ(2U, packets.size());
    ASSERT_TRUE(packets[0]->instance_of<IntegerPacket>());
    ASSERT_EQ(10, packets[0]->packet_cast<IntegerPacket>()->getValue());
    ASSERT_TRUE(packets[1]->instance_of<SimpleStringPacket>());
    ASSERT_EQ("OK", packets[1]->packet_cast<SimpleStringPacket>()->getValue());

    PushPacket push11;
    buf.clear();
    buf.append(">");
    ASSERT_EQ(DState::AGAIN, push11.decodeRESP3(&buf));
    buf.append("2\r\n");
    ASSERT_EQ(DState::AGAIN, push11.decodeRESP3(&buf));
    buf.append(":");
    ASSERT_EQ(DState::AGAIN, push11.decodeRESP3(&buf));
    buf.append("10\r\n");
    ASSERT_EQ(DState::AGAIN, push11.decodeRESP3(&buf));
    buf.append(",");
    ASSERT_EQ(DState::AGAIN, push11.decodeRESP3(&buf));
    buf.append("1.2\r\n");
    ASSERT_EQ(DState::SUCCESS, push11.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(15, push11.getRESP3EncodeSize());
    packets = push11.getPacketArray();
    ASSERT_EQ(2U, packets.size());
    ASSERT_TRUE(packets[0]->instance_of<IntegerPacket>());
    ASSERT_EQ(10, packets[0]->packet_cast<IntegerPacket>()->getValue());
    ASSERT_TRUE(packets[1]->instance_of<DoublePacket>());
    ASSERT_DOUBLE_EQ(1.2, packets[1]->packet_cast<DoublePacket>()->getValue());
}
