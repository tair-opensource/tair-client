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
#include "protocol/packet/resp/AttributePacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::IntegerPacket;
using tair::protocol::SimpleStringPacket;
using tair::protocol::BulkStringPacket;
using tair::protocol::ErrorPacket;
using tair::protocol::DoublePacket;
using tair::protocol::AttributePacket;

TEST(ATTRIBUTE_PACKET_TEST, ENCODE_TEST) {
    AttributePacket attribute1;
    Buffer buf;
    attribute1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), attribute1.getRESP3EncodeSize());
    ASSERT_EQ("|0\r\n", buf.nextAllString());
    buf.clear();
    attribute1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), attribute1.getRESP2EncodeSize());
    ASSERT_EQ("*0\r\n", buf.nextAllString());

    AttributePacket attribute2;
    buf.clear();
    attribute2.addPacketPair(new IntegerPacket(1), new IntegerPacket(2));
    attribute2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), attribute2.getRESP3EncodeSize());
    ASSERT_EQ("|1\r\n:1\r\n:2\r\n", buf.nextAllString());
    buf.clear();
    attribute2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), attribute2.getRESP2EncodeSize());
    ASSERT_EQ("*2\r\n:1\r\n:2\r\n", buf.nextAllString());

    AttributePacket attribute3(std::vector<std::string> {"k1", "v1", "k2", "v2"});
    buf.clear();
    attribute3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), attribute3.getRESP3EncodeSize());
    ASSERT_EQ("|2\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n$2\r\nv2\r\n", buf.nextAllString());
    buf.clear();
    attribute3.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), attribute3.getRESP2EncodeSize());
    ASSERT_EQ("*4\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n$2\r\nv2\r\n", buf.nextAllString());

    AttributePacket attribute4(std::vector<int64_t> {1, 2, 3, 4});
    buf.clear();
    attribute4.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), attribute4.getRESP3EncodeSize());
    ASSERT_EQ("|2\r\n:1\r\n:2\r\n:3\r\n:4\r\n", buf.nextAllString());
    buf.clear();
    attribute4.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), attribute4.getRESP2EncodeSize());
    ASSERT_EQ("*4\r\n:1\r\n:2\r\n:3\r\n:4\r\n", buf.nextAllString());

    AttributePacket attribute5;
    attribute5.addReplyBulkBulk("k1", "v1");
    attribute5.addReplyBulkInteger("k2", 2);
    buf.clear();
    attribute5.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), attribute5.getRESP3EncodeSize());
    ASSERT_EQ("|2\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n", buf.nextAllString());
    buf.clear();
    attribute5.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), attribute5.getRESP2EncodeSize());
    ASSERT_EQ("*4\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n", buf.nextAllString());

    AttributePacket attribute6;
    attribute6.addReplyBulkBulk("k1", "v1");
    attribute6.addReplyBulkInteger("k2", 2);
    attribute6.addReplyBulkArray("k3", std::vector<std::string>());
    buf.clear();
    attribute6.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), attribute6.getRESP3EncodeSize());
    ASSERT_EQ("|3\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*0\r\n", buf.nextAllString());
    buf.clear();
    attribute6.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), attribute6.getRESP2EncodeSize());
    ASSERT_EQ("*6\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*0\r\n", buf.nextAllString());

    AttributePacket attribute7;
    attribute7.addReplyBulkBulk("k1", "v1");
    attribute7.addReplyBulkInteger("k2", 2);
    attribute7.addReplyBulkArray("k3", std::vector<std::string> {"a", "b", "c"});
    buf.clear();
    attribute7.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), attribute7.getRESP3EncodeSize());
    ASSERT_EQ("|3\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*3\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\nc\r\n", buf.nextAllString());
    buf.clear();
    attribute7.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), attribute7.getRESP2EncodeSize());
    ASSERT_EQ("*6\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*3\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\nc\r\n", buf.nextAllString());

    AttributePacket attribute8;
    attribute8.addReplyBulkBulk("k1", "v1");
    attribute8.addReplyBulkInteger("k2", 2);
    attribute8.addReplyBulkSet("k3", std::vector<std::string>());
    buf.clear();
    attribute8.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), attribute8.getRESP3EncodeSize());
    ASSERT_EQ("|3\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n~0\r\n", buf.nextAllString());
    buf.clear();
    attribute8.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), attribute8.getRESP2EncodeSize());
    ASSERT_EQ("*6\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*0\r\n", buf.nextAllString());

    AttributePacket attribute9;
    attribute9.addReplyBulkBulk("k1", "v1");
    attribute9.addReplyBulkInteger("k2", 2);
    attribute9.addReplyBulkSet("k3", std::vector<std::string> {"a", "b", "c"});
    buf.clear();
    attribute9.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), attribute9.getRESP3EncodeSize());
    ASSERT_EQ("|3\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n~3\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\nc\r\n", buf.nextAllString());
    buf.clear();
    attribute9.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), attribute9.getRESP2EncodeSize());
    ASSERT_EQ("*6\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*3\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\nc\r\n", buf.nextAllString());
}

TEST(ATTRIBUTE_PACKET_TEST, DECODE_TEST) {
    AttributePacket attribute1;
    Buffer buf;
    ASSERT_EQ(DState::ERROR, attribute1.decodeRESP2(&buf));
    ASSERT_EQ(DState::AGAIN, attribute1.decodeRESP3(&buf));
    buf.append(":333\r\n");
    ASSERT_EQ(DState::ERROR, attribute1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '|', got ':'", attribute1.getDecodeErr());

    AttributePacket attribute2;
    buf.clear();

    buf.append("|");
    buf.append(std::string(1024 * 65, 'a'));
    ASSERT_EQ(DState::ERROR, attribute2.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: too big count string", attribute2.getDecodeErr());

    AttributePacket attribute3;
    buf.clear();
    buf.append("|\r\n");
    ASSERT_EQ(DState::ERROR, attribute3.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found array size", attribute3.getDecodeErr());

    AttributePacket attribute4;
    buf.clear();
    buf.append("|123abc\r\n");
    ASSERT_EQ(DState::ERROR, attribute4.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: integer format error", attribute4.getDecodeErr());

    AttributePacket attribute5;
    buf.clear();
    buf.append("|-2\r\n");
    ASSERT_EQ(DState::ERROR, attribute5.decodeRESP3(&buf));

    AttributePacket attribute6;
    buf.clear();
    buf.append("|-1\r\n");
    ASSERT_EQ(DState::ERROR, attribute6.decodeRESP3(&buf));

    AttributePacket attribute7;
    buf.clear();
    buf.append("|1\r\n");
    ASSERT_EQ(DState::AGAIN, attribute7.decodeRESP3(&buf));
    buf.append("-test\r\n-test\r\n");
    ASSERT_EQ(DState::SUCCESS, attribute7.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(18, attribute7.getRESP2EncodeSize());

    AttributePacket attribute8;
    buf.clear();
    buf.append("|1\r\n");
    ASSERT_EQ(DState::AGAIN, attribute8.decodeRESP3(&buf));
    buf.append("&test\r\n");
    ASSERT_EQ(DState::ERROR, attribute8.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: unknown packet type: '&'", attribute8.getDecodeErr());

    AttributePacket attribute9;
    buf.clear();
    buf.append("|1\r\n");
    ASSERT_EQ(DState::AGAIN, attribute9.decodeRESP3(&buf));
    buf.append(":\r\n");
    ASSERT_EQ(DState::ERROR, attribute9.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found integer", attribute9.getDecodeErr());

    AttributePacket attribute10;
    buf.clear();
    buf.append("|");
    ASSERT_EQ(DState::AGAIN, attribute10.decodeRESP3(&buf));
    buf.append("2\r\n");
    ASSERT_EQ(DState::AGAIN, attribute10.decodeRESP3(&buf));
    buf.append(":");
    ASSERT_EQ(DState::AGAIN, attribute10.decodeRESP3(&buf));
    buf.append("10\r\n");
    ASSERT_EQ(DState::AGAIN, attribute10.decodeRESP3(&buf));
    buf.append("+");
    ASSERT_EQ(DState::AGAIN, attribute10.decodeRESP3(&buf));
    buf.append("OK\r\n");
    ASSERT_EQ(DState::AGAIN, attribute10.decodeRESP3(&buf));
    buf.append("$4\r\n");
    ASSERT_EQ(DState::AGAIN, attribute10.decodeRESP3(&buf));
    buf.append("test\r\n");
    ASSERT_EQ(DState::AGAIN, attribute10.decodeRESP3(&buf));
    buf.append(",");
    ASSERT_EQ(DState::AGAIN, attribute10.decodeRESP3(&buf));
    buf.append("1.2\r\n");
    ASSERT_EQ(DState::SUCCESS, attribute10.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(33, attribute10.getRESP2EncodeSize());
    auto packet_pairs = attribute10.getPacketArray();
    ASSERT_EQ(2U, packet_pairs.size());
    ASSERT_TRUE(packet_pairs[0].first->instance_of<IntegerPacket>());
    ASSERT_EQ(10, packet_pairs[0].first->packet_cast<IntegerPacket>()->getValue());
    ASSERT_TRUE(packet_pairs[0].second->instance_of<SimpleStringPacket>());
    ASSERT_EQ("OK", packet_pairs[0].second->packet_cast<SimpleStringPacket>()->getValue());
    ASSERT_TRUE(packet_pairs[1].first->instance_of<BulkStringPacket>());
    ASSERT_EQ("test", packet_pairs[1].first->packet_cast<BulkStringPacket>()->getValue());
    ASSERT_TRUE(packet_pairs[1].second->instance_of<DoublePacket>());
    ASSERT_DOUBLE_EQ(1.2, packet_pairs[1].second->packet_cast<DoublePacket>()->getValue());
}
