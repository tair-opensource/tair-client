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
#include "protocol/packet/resp/MapPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::IntegerPacket;
using tair::protocol::SimpleStringPacket;
using tair::protocol::BulkStringPacket;
using tair::protocol::ErrorPacket;
using tair::protocol::DoublePacket;
using tair::protocol::MapPacket;

TEST(MAP_PACKET_TEST, LVALUE_REFERENCE_TEST) {
    std::vector<std::string> bulks {"k1", "k2", "k3", "k4"};
    MapPacket map1(bulks);
    ASSERT_EQ(bulks.size() / 2, map1.getPacketArray().size());
    ASSERT_EQ("k1", bulks[0]);
    ASSERT_EQ("k2", bulks[1]);
    ASSERT_EQ("k3", bulks[2]);
    ASSERT_EQ("k4", bulks[3]);
}

TEST(MAP_PACKET_TEST, RVALUE_REFERENCE_TEST) {
    std::vector<std::string> bulks {"k1", "k2", "k3", "k4"};
    MapPacket map1(std::move(bulks));
    ASSERT_EQ(bulks.size() / 2, map1.getPacketArray().size());
    ASSERT_EQ("", bulks[0]);
    ASSERT_EQ("", bulks[1]);
    ASSERT_EQ("", bulks[2]);
    ASSERT_EQ("", bulks[3]);
}

TEST(MAP_PACKET_TEST, ENCODE_TEST) {
    MapPacket map1;
    Buffer buf;
    map1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), map1.getRESP3EncodeSize());
    ASSERT_EQ("%0\r\n", buf.nextAllString());
    buf.clear();
    map1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), map1.getRESP2EncodeSize());
    ASSERT_EQ("*0\r\n", buf.nextAllString());

    MapPacket map2;
    buf.clear();
    map2.addPacketPair(new IntegerPacket(1), new IntegerPacket(2));
    map2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), map2.getRESP3EncodeSize());
    ASSERT_EQ("%1\r\n:1\r\n:2\r\n", buf.nextAllString());
    buf.clear();
    map2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), map2.getRESP2EncodeSize());
    ASSERT_EQ("*2\r\n:1\r\n:2\r\n", buf.nextAllString());

    MapPacket map3(std::vector<std::string> {"k1", "v1", "k2", "v2"});
    buf.clear();
    map3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), map3.getRESP3EncodeSize());
    ASSERT_EQ("%2\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n$2\r\nv2\r\n", buf.nextAllString());
    buf.clear();
    map3.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), map3.getRESP2EncodeSize());
    ASSERT_EQ("*4\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n$2\r\nv2\r\n", buf.nextAllString());

    MapPacket map4(std::vector<int64_t> {1, 2, 3, 4});
    buf.clear();
    map4.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), map4.getRESP3EncodeSize());
    ASSERT_EQ("%2\r\n:1\r\n:2\r\n:3\r\n:4\r\n", buf.nextAllString());
    buf.clear();
    map4.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), map4.getRESP2EncodeSize());
    ASSERT_EQ("*4\r\n:1\r\n:2\r\n:3\r\n:4\r\n", buf.nextAllString());

    MapPacket map5(std::vector<double> {1.2, 1.2, 1.2, 1.2});
    buf.clear();
    map5.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), map5.getRESP3EncodeSize());
    ASSERT_EQ("%2\r\n,1.2\r\n,1.2\r\n,1.2\r\n,1.2\r\n", buf.nextAllString());
    buf.clear();
    map5.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), map5.getRESP2EncodeSize());
    ASSERT_EQ("*4\r\n$3\r\n1.2\r\n$3\r\n1.2\r\n$3\r\n1.2\r\n$3\r\n1.2\r\n", buf.nextAllString());

    MapPacket map6;
    map6.addReplyBulkBulk("k1", "v1");
    map6.addReplyBulkInteger("k2", 2);
    buf.clear();
    map6.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), map6.getRESP3EncodeSize());
    ASSERT_EQ("%2\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n", buf.nextAllString());
    buf.clear();
    map6.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), map6.getRESP2EncodeSize());
    ASSERT_EQ("*4\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n", buf.nextAllString());

    MapPacket map7;
    map7.addReplyBulkBulk("k1", "v1");
    map7.addReplyBulkInteger("k2", 2);
    map7.addReplyBulkArray("k3", std::vector<std::string>());
    buf.clear();
    map7.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), map7.getRESP3EncodeSize());
    ASSERT_EQ("%3\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*0\r\n", buf.nextAllString());
    buf.clear();
    map7.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), map7.getRESP2EncodeSize());
    ASSERT_EQ("*6\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*0\r\n", buf.nextAllString());

    MapPacket map8;
    map8.addReplyBulkBulk("k1", "v1");
    map8.addReplyBulkInteger("k2", 2);
    map8.addReplyBulkArray("k3", std::vector<std::string> {"a", "b", "c"});
    buf.clear();
    map8.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), map8.getRESP3EncodeSize());
    ASSERT_EQ("%3\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*3\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\nc\r\n", buf.nextAllString());
    buf.clear();
    map8.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), map8.getRESP2EncodeSize());
    ASSERT_EQ("*6\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*3\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\nc\r\n", buf.nextAllString());

    MapPacket map9;
    map9.addReplyBulkBulk("k1", "v1");
    map9.addReplyBulkInteger("k2", 2);
    map9.addReplyBulkSet("k3", std::vector<std::string>());
    buf.clear();
    map9.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), map9.getRESP3EncodeSize());
    ASSERT_EQ("%3\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n~0\r\n", buf.nextAllString());
    buf.clear();
    map9.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), map9.getRESP2EncodeSize());
    ASSERT_EQ("*6\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*0\r\n", buf.nextAllString());

    MapPacket map10;
    map10.addReplyBulkBulk("k1", "v1");
    map10.addReplyBulkInteger("k2", 2);
    map10.addReplyBulkSet("k3", std::vector<std::string> {"a", "b", "c"});
    buf.clear();
    map10.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), map10.getRESP3EncodeSize());
    ASSERT_EQ("%3\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n~3\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\nc\r\n", buf.nextAllString());
    buf.clear();
    map10.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), map10.getRESP2EncodeSize());
    ASSERT_EQ("*6\r\n$2\r\nk1\r\n$2\r\nv1\r\n$2\r\nk2\r\n:2\r\n$2\r\nk3\r\n*3\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\nc\r\n", buf.nextAllString());
}

TEST(MAP_PACKET_TEST, DECODE_TEST) {
    MapPacket map1;
    Buffer buf;
    ASSERT_EQ(DState::ERROR, map1.decodeRESP2(&buf));
    ASSERT_EQ(DState::AGAIN, map1.decodeRESP3(&buf));
    buf.append(":333\r\n");
    ASSERT_EQ(DState::ERROR, map1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '%', got ':'", map1.getDecodeErr());

    MapPacket map2;
    buf.clear();

    buf.append("%");
    buf.append(std::string(1024 * 65, 'a'));
    ASSERT_EQ(DState::ERROR, map2.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: too big count string", map2.getDecodeErr());

    MapPacket map3;
    buf.clear();
    buf.append("%\r\n");
    ASSERT_EQ(DState::ERROR, map3.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found array size", map3.getDecodeErr());

    MapPacket map4;
    buf.clear();
    buf.append("%123abc\r\n");
    ASSERT_EQ(DState::ERROR, map4.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: integer format error", map4.getDecodeErr());

    MapPacket map5;
    buf.clear();
    buf.append("%-2\r\n");
    ASSERT_EQ(DState::ERROR, map5.decodeRESP3(&buf));

    MapPacket map6;
    buf.clear();
    buf.append("%-1\r\n");
    ASSERT_EQ(DState::ERROR, map6.decodeRESP3(&buf));

    MapPacket map7;
    buf.clear();
    buf.append("%1\r\n");
    ASSERT_EQ(DState::AGAIN, map7.decodeRESP3(&buf));
    buf.append("-test\r\n-test\r\n");
    ASSERT_EQ(DState::SUCCESS, map7.decodeRESP3(&buf));
    ASSERT_EQ(18, map7.getRESP2EncodeSize());

    MapPacket map8;
    buf.clear();
    buf.append("%1\r\n");
    ASSERT_EQ(DState::AGAIN, map8.decodeRESP3(&buf));
    buf.append("&test\r\n");
    ASSERT_EQ(DState::ERROR, map8.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: unknown packet type: '&'", map8.getDecodeErr());

    MapPacket map9;
    buf.clear();
    buf.append("%1\r\n");
    ASSERT_EQ(DState::AGAIN, map9.decodeRESP3(&buf));
    buf.append(":\r\n");
    ASSERT_EQ(DState::ERROR, map9.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found integer", map9.getDecodeErr());

    MapPacket map10;
    buf.clear();
    buf.append("%");
    ASSERT_EQ(DState::AGAIN, map10.decodeRESP3(&buf));
    buf.append("2\r\n");
    ASSERT_EQ(DState::AGAIN, map10.decodeRESP3(&buf));
    buf.append(":");
    ASSERT_EQ(DState::AGAIN, map10.decodeRESP3(&buf));
    buf.append("10\r\n");
    ASSERT_EQ(DState::AGAIN, map10.decodeRESP3(&buf));
    buf.append("+");
    ASSERT_EQ(DState::AGAIN, map10.decodeRESP3(&buf));
    buf.append("OK\r\n");
    ASSERT_EQ(DState::AGAIN, map10.decodeRESP3(&buf));
    buf.append("$4\r\n");
    ASSERT_EQ(DState::AGAIN, map10.decodeRESP3(&buf));
    buf.append("test\r\n");
    ASSERT_EQ(DState::AGAIN, map10.decodeRESP3(&buf));
    buf.append(",");
    ASSERT_EQ(DState::AGAIN, map10.decodeRESP3(&buf));
    buf.append("1.2\r\n");
    ASSERT_EQ(DState::SUCCESS, map10.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(33, map10.getRESP2EncodeSize());
    auto packet_pairs = map10.getPacketArray();
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
