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
#include "protocol/packet/resp/SetPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::IntegerPacket;
using tair::protocol::SimpleStringPacket;
using tair::protocol::DoublePacket;
using tair::protocol::SetPacket;

TEST(SET_PACKET_TEST, ENCODE_TEST) {
    SetPacket set1;
    Buffer buf;
    set1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), set1.getRESP3EncodeSize());
    ASSERT_EQ("~0\r\n", buf.nextAllString());
    buf.clear();
    set1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), set1.getRESP2EncodeSize());
    ASSERT_EQ("*0\r\n", buf.nextAllString());

    SetPacket set2;
    buf.clear();
    set2.addReplyInteger(10);
    set2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), set2.getRESP3EncodeSize());
    ASSERT_EQ("~1\r\n:10\r\n", buf.nextAllString());
    buf.clear();
    set2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), set2.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n:10\r\n", buf.nextAllString());

    SetPacket set3;
    buf.clear();
    set3.addReplyStatus("status");
    set3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), set3.getRESP3EncodeSize());
    ASSERT_EQ("~1\r\n+status\r\n", buf.nextAllString());
    buf.clear();
    set3.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), set3.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n+status\r\n", buf.nextAllString());

    SetPacket set4;
    buf.clear();
    set4.addReplyBulk("bulk");
    set4.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), set4.getRESP3EncodeSize());
    ASSERT_EQ("~1\r\n$4\r\nbulk\r\n", buf.nextAllString());
    buf.clear();
    set4.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), set4.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n$4\r\nbulk\r\n", buf.nextAllString());

    std::vector<int64_t> argv {1, 2, 3};
    SetPacket set5(argv);
    buf.clear();
    set5.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), set5.getRESP3EncodeSize());
    ASSERT_EQ("~3\r\n:1\r\n:2\r\n:3\r\n", buf.nextAllString());
    buf.clear();
    set5.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), set5.getRESP2EncodeSize());
    ASSERT_EQ("*3\r\n:1\r\n:2\r\n:3\r\n", buf.nextAllString());
    argv.clear();
    std::vector<std::string> test_argv;
    ASSERT_FALSE(set5.moveBulks(test_argv));
    ASSERT_TRUE(set5.moveIntegers(argv));
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ(1, argv[0]);
    ASSERT_EQ(2, argv[1]);
    ASSERT_EQ(3, argv[2]);

    std::vector<std::string> argv2 {"set", "key", "value"};
    SetPacket set6(argv2);
    buf.clear();
    set6.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), set6.getRESP3EncodeSize());
    ASSERT_EQ("~3\r\n$3\r\nset\r\n$3\r\nkey\r\n$5\r\nvalue\r\n", buf.nextAllString());
    buf.clear();
    set6.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), set6.getRESP2EncodeSize());
    ASSERT_EQ("*3\r\n$3\r\nset\r\n$3\r\nkey\r\n$5\r\nvalue\r\n", buf.nextAllString());
    argv2.clear();
    set6.moveBulks(argv2);
    ASSERT_EQ(3U, argv2.size());
    ASSERT_EQ(3U, argv2[0].size());
    ASSERT_EQ(3U, argv2[1].size());
    ASSERT_EQ(5U, argv2[2].size());

    SetPacket set7;
    set7.addReplyDouble(1.2);
    buf.clear();
    set7.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), set7.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n$3\r\n1.2\r\n", buf.nextAllString());
    buf.clear();
    set7.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), set7.getRESP3EncodeSize());
    ASSERT_EQ("~1\r\n,1.2\r\n", buf.nextAllString());
}

TEST(SET_PACKET_TEST, DECODE_TEST) {
    SetPacket set1;
    Buffer buf;
    ASSERT_EQ(DState::ERROR, set1.decodeRESP2(&buf));
    ASSERT_EQ(DState::AGAIN, set1.decodeRESP3(&buf));
    buf.append(":333\r\n");
    ASSERT_EQ(DState::ERROR, set1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '~', got ':'", set1.getDecodeErr());

    SetPacket set2;
    buf.clear();
    buf.append("~");
    buf.append(std::string(1024 * 65, 'a'));
    ASSERT_EQ(DState::ERROR, set2.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: too big count string", set2.getDecodeErr());

    SetPacket set3;
    buf.clear();
    buf.append("~\r\n");
    ASSERT_EQ(DState::ERROR, set3.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found array size", set3.getDecodeErr());

    SetPacket set4;
    buf.clear();
    buf.append("~123abc\r\n");
    ASSERT_EQ(DState::ERROR, set4.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: integer format error", set4.getDecodeErr());

    SetPacket set5;
    buf.clear();
    buf.append("~-2\r\n");
    ASSERT_EQ(DState::ERROR, set5.decodeRESP3(&buf));

    SetPacket set6;
    buf.clear();
    buf.append("~-1\r\n");
    ASSERT_EQ(DState::ERROR, set6.decodeRESP3(&buf));

    SetPacket set7;
    buf.clear();
    buf.append("~1\r\n");
    ASSERT_EQ(DState::AGAIN, set7.decodeRESP3(&buf));
    buf.append("-test\r\n");
    ASSERT_EQ(DState::SUCCESS, set7.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(11, set7.getRESP2EncodeSize());

    SetPacket set8;
    buf.clear();
    buf.append("~1\r\n");
    ASSERT_EQ(DState::AGAIN, set8.decodeRESP3(&buf));
    buf.append("&test\r\n");
    ASSERT_EQ(DState::ERROR, set8.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: unknown packet type: '&'", set8.getDecodeErr());

    SetPacket set9;
    buf.clear();
    buf.append("~1\r\n");
    ASSERT_EQ(DState::AGAIN, set9.decodeRESP3(&buf));
    buf.append(":\r\n");
    ASSERT_EQ(DState::ERROR, set9.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found integer", set9.getDecodeErr());

    SetPacket set10;
    buf.clear();
    buf.append("~");
    ASSERT_EQ(DState::AGAIN, set10.decodeRESP3(&buf));
    buf.append("2\r\n");
    ASSERT_EQ(DState::AGAIN, set10.decodeRESP3(&buf));
    buf.append(":");
    ASSERT_EQ(DState::AGAIN, set10.decodeRESP3(&buf));
    buf.append("10\r\n");
    ASSERT_EQ(DState::AGAIN, set10.decodeRESP3(&buf));
    buf.append("+");
    ASSERT_EQ(DState::AGAIN, set10.decodeRESP3(&buf));
    buf.append("OK\r\n");
    ASSERT_EQ(DState::SUCCESS, set10.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(14, set10.getRESP2EncodeSize());
    auto packets = set10.getPacketArray();
    ASSERT_EQ(2U, packets.size());
    ASSERT_TRUE(packets[0]->instance_of<IntegerPacket>());
    ASSERT_EQ(10, packets[0]->packet_cast<IntegerPacket>()->getValue());
    ASSERT_TRUE(packets[1]->instance_of<SimpleStringPacket>());
    ASSERT_EQ("OK", packets[1]->packet_cast<SimpleStringPacket>()->getValue());

    SetPacket set11;
    buf.clear();
    buf.append("~");
    ASSERT_EQ(DState::AGAIN, set11.decodeRESP3(&buf));
    buf.append("2\r\n");
    ASSERT_EQ(DState::AGAIN, set11.decodeRESP3(&buf));
    buf.append(",");
    ASSERT_EQ(DState::AGAIN, set11.decodeRESP3(&buf));
    buf.append("1.2\r\n");
    ASSERT_EQ(DState::AGAIN, set11.decodeRESP3(&buf));
    buf.append(",");
    ASSERT_EQ(DState::AGAIN, set11.decodeRESP3(&buf));
    buf.append("1.2\r\n");
    ASSERT_EQ(DState::SUCCESS, set11.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(16, set11.getRESP3EncodeSize());
    packets = set11.getPacketArray();
    ASSERT_EQ(2U, packets.size());
    ASSERT_TRUE(packets[0]->instance_of<DoublePacket>());
    ASSERT_DOUBLE_EQ(1.2, packets[0]->packet_cast<DoublePacket>()->getValue());
    ASSERT_TRUE(packets[1]->instance_of<DoublePacket>());
    ASSERT_DOUBLE_EQ(1.2, packets[1]->packet_cast<DoublePacket>()->getValue());
}
