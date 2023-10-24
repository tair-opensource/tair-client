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
#include "protocol/packet/resp/ArrayPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::PacketType;
using tair::protocol::Packet;
using tair::protocol::IntegerPacket;
using tair::protocol::SimpleStringPacket;
using tair::protocol::BulkStringPacket;
using tair::protocol::DoublePacket;
using tair::protocol::ArrayPacket;

TEST(ARRAY_PACKET_TEST, LVALUE_REFERENCE_TEST) {
    std::vector<std::string> bulks {"k1", "k2", "k3"};
    ArrayPacket array(bulks);
    ASSERT_EQ(bulks.size(), array.getPacketArray().size());
    ASSERT_EQ("k1", bulks[0]);
    ASSERT_EQ("k2", bulks[1]);
    ASSERT_EQ("k3", bulks[2]);
}

TEST(ARRAY_PACKET_TEST, RVALUE_REFERENCE_TEST) {
    std::vector<std::string> bulks {"k1", "k2", "k3"};
    ArrayPacket array(std::move(bulks));
    ASSERT_EQ(bulks.size(), array.getPacketArray().size());
    ASSERT_EQ("", bulks[0]);
    ASSERT_EQ("", bulks[1]);
    ASSERT_EQ("", bulks[2]);
}

TEST(ARRAY_PACKET_TEST, ENCODE_TEST) {
    ArrayPacket array1(PacketType::TYPE_NULL);
    Buffer buf;
    array1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array1.getRESP2EncodeSize());
    ASSERT_EQ("*-1\r\n", buf.nextAllString());
    buf.clear();
    array1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array1.getRESP3EncodeSize());
    ASSERT_EQ("*-1\r\n", buf.nextAllString());

    ArrayPacket array2;
    buf.clear();
    array2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array2.getRESP2EncodeSize());
    ASSERT_EQ("*0\r\n", buf.nextAllString());
    buf.clear();
    array2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array2.getRESP3EncodeSize());
    ASSERT_EQ("*0\r\n", buf.nextAllString());

    ArrayPacket array3;
    buf.clear();
    array3.addReplyInteger(10);
    array3.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array3.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n:10\r\n", buf.nextAllString());
    buf.clear();
    array3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array3.getRESP3EncodeSize());
    ASSERT_EQ("*1\r\n:10\r\n", buf.nextAllString());

    ArrayPacket array4;
    buf.clear();
    array4.addReplyStatus("status");
    array4.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array4.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n+status\r\n", buf.nextAllString());
    buf.clear();
    array4.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array4.getRESP3EncodeSize());
    ASSERT_EQ("*1\r\n+status\r\n", buf.nextAllString());

    ArrayPacket array5;
    buf.clear();
    array5.addReplyBulk("bulk");
    array5.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array5.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n$4\r\nbulk\r\n", buf.nextAllString());
    buf.clear();
    array5.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array5.getRESP3EncodeSize());
    ASSERT_EQ("*1\r\n$4\r\nbulk\r\n", buf.nextAllString());

    ArrayPacket array6;
    buf.clear();
    array6.addReplyBulk("bulk");
    ArrayPacket *sub_array = array6.startNewArray();
    sub_array->addReplyBulk("subbulk");
    sub_array->addReplyStatus("status");
    array6.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array6.getRESP2EncodeSize());
    ASSERT_EQ("*2\r\n$4\r\nbulk\r\n*2\r\n$7\r\nsubbulk\r\n+status\r\n", buf.nextAllString());
    buf.clear();
    array6.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array6.getRESP3EncodeSize());
    ASSERT_EQ("*2\r\n$4\r\nbulk\r\n*2\r\n$7\r\nsubbulk\r\n+status\r\n", buf.nextAllString());

    std::vector<std::string> argv {"set", "key", "value"};
    ArrayPacket array7(argv);
    buf.clear();
    array7.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array7.getRESP2EncodeSize());
    ASSERT_EQ("*3\r\n$3\r\nset\r\n$3\r\nkey\r\n$5\r\nvalue\r\n", buf.nextAllString());
    buf.clear();
    array7.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array7.getRESP3EncodeSize());
    ASSERT_EQ("*3\r\n$3\r\nset\r\n$3\r\nkey\r\n$5\r\nvalue\r\n", buf.nextAllString());

    ArrayPacket array8(std::move(argv));
    buf.clear();
    array8.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array8.getRESP2EncodeSize());
    ASSERT_EQ("*3\r\n$3\r\nset\r\n$3\r\nkey\r\n$5\r\nvalue\r\n", buf.nextAllString());
    buf.clear();
    array8.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array8.getRESP3EncodeSize());
    ASSERT_EQ("*3\r\n$3\r\nset\r\n$3\r\nkey\r\n$5\r\nvalue\r\n", buf.nextAllString());
    argv.clear();
    std::vector<int64_t> test_argv;
    ASSERT_FALSE(array8.moveIntegers(test_argv));
    ASSERT_TRUE(array8.moveBulks(argv));
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ(3U, argv[0].size());
    ASSERT_EQ(3U, argv[1].size());
    ASSERT_EQ(5U, argv[2].size());

    ArrayPacket array9;
    array9.addReplyNull();
    array9.addReplyNullArray();
    buf.clear();
    array9.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array9.getRESP2EncodeSize());
    ASSERT_EQ("*2\r\n$-1\r\n*-1\r\n", buf.nextAllString());
    buf.clear();
    array9.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array9.getRESP3EncodeSize());
    ASSERT_EQ("*2\r\n_\r\n_\r\n", buf.nextAllString());

    ArrayPacket array10;
    std::vector<Packet *> packets;
    packets.emplace_back(new IntegerPacket(1));
    packets.emplace_back(new BulkStringPacket("he"));
    array10.addPackets(packets);
    buf.clear();
    array10.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array10.getRESP2EncodeSize());
    ASSERT_EQ("*2\r\n:1\r\n$2\r\nhe\r\n", buf.nextAllString());
    buf.clear();
    array10.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array10.getRESP3EncodeSize());
    ASSERT_EQ("*2\r\n:1\r\n$2\r\nhe\r\n", buf.nextAllString());

    ArrayPacket array11;
    array11.addReplyDouble(1.2);
    buf.clear();
    array11.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), array11.getRESP2EncodeSize());
    ASSERT_EQ("*1\r\n$3\r\n1.2\r\n", buf.nextAllString());
    buf.clear();
    array11.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), array11.getRESP3EncodeSize());
    ASSERT_EQ("*1\r\n,1.2\r\n", buf.nextAllString());
}

TEST(ARRAY_PACKET_TEST, DECODE_TEST) {
    ArrayPacket arrayPacket1;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, arrayPacket1.decodeRESP2(&buf));
    buf.append(":333\r\n");
    ASSERT_EQ(DState::ERROR, arrayPacket1.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: expected '*', got ':'", arrayPacket1.getDecodeErr());

    ArrayPacket arrayPacket2;
    buf.clear();
    buf.append("*");
    buf.append(std::string(1024 * 65, 'a'));
    ASSERT_EQ(DState::ERROR, arrayPacket2.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: too big count string", arrayPacket2.getDecodeErr());

    ArrayPacket arrayPacket3;
    buf.clear();
    buf.append("*\r\n");
    ASSERT_EQ(DState::ERROR, arrayPacket3.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: not found array size", arrayPacket3.getDecodeErr());

    ArrayPacket arrayPacket4;
    buf.clear();
    buf.append("*123abc\r\n");
    ASSERT_EQ(DState::ERROR, arrayPacket4.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: integer format error", arrayPacket4.getDecodeErr());

    ArrayPacket arrayPacket5;
    buf.clear();
    buf.append("*-2\r\n");
    ASSERT_EQ(DState::SUCCESS, arrayPacket5.decodeRESP2(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(PacketType::TYPE_NULL, arrayPacket5.getType());
    ASSERT_EQ(5, arrayPacket5.getRESP2EncodeSize());

    ArrayPacket arrayPacket6;
    buf.clear();
    buf.append("*-1\r\n");
    ASSERT_EQ(DState::SUCCESS, arrayPacket6.decodeRESP2(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(PacketType::TYPE_NULL, arrayPacket6.getType());
    ASSERT_EQ(5, arrayPacket6.getRESP2EncodeSize());

    ArrayPacket arrayPacket7;
    buf.clear();
    buf.append("*1\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket7.decodeRESP2(&buf));
    buf.append("-test\r\n");
    ASSERT_EQ(DState::SUCCESS, arrayPacket7.decodeRESP2(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(11, arrayPacket7.getRESP2EncodeSize());

    ArrayPacket arrayPacket8;
    buf.clear();
    buf.append("*1\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket8.decodeRESP2(&buf));
    buf.append("&test\r\n");
    ASSERT_EQ(DState::ERROR, arrayPacket8.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: unknown packet type: '&'", arrayPacket8.getDecodeErr());

    ArrayPacket arrayPacket9;
    buf.clear();
    buf.append("*1\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket9.decodeRESP2(&buf));
    buf.append(":\r\n");
    ASSERT_EQ(DState::ERROR, arrayPacket9.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: not found integer", arrayPacket9.getDecodeErr());

    ArrayPacket arrayPacket10;
    buf.clear();
    buf.append("*");
    ASSERT_EQ(DState::AGAIN, arrayPacket10.decodeRESP2(&buf));
    buf.append("2\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket10.decodeRESP2(&buf));
    buf.append(":");
    ASSERT_EQ(DState::AGAIN, arrayPacket10.decodeRESP2(&buf));
    buf.append("10\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket10.decodeRESP2(&buf));
    buf.append("+");
    ASSERT_EQ(DState::AGAIN, arrayPacket10.decodeRESP2(&buf));
    buf.append("OK\r\n");
    ASSERT_EQ(DState::SUCCESS, arrayPacket10.decodeRESP2(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(14, arrayPacket10.getRESP2EncodeSize());
    auto packets = arrayPacket10.getPacketArray();
    ASSERT_EQ(2U, packets.size());
    ASSERT_TRUE(packets[0]->instance_of<IntegerPacket>());
    ASSERT_EQ(10, packets[0]->packet_cast<IntegerPacket>()->getValue());
    ASSERT_TRUE(packets[1]->instance_of<SimpleStringPacket>());
    ASSERT_EQ("OK", packets[1]->packet_cast<SimpleStringPacket>()->getValue());
}

TEST(ARRAY_PACKET_TEST, DECODE3_TEST) {
    ArrayPacket arrayPacket1;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, arrayPacket1.decodeRESP3(&buf));
    buf.append(":333\r\n");
    ASSERT_EQ(DState::ERROR, arrayPacket1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '*', got ':'", arrayPacket1.getDecodeErr());

    ArrayPacket arrayPacket2;
    buf.clear();
    buf.append("*");
    buf.append(std::string(1024 * 65, 'a'));
    ASSERT_EQ(DState::ERROR, arrayPacket2.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: too big count string", arrayPacket2.getDecodeErr());

    ArrayPacket arrayPacket3;
    buf.clear();
    buf.append("*\r\n");
    ASSERT_EQ(DState::ERROR, arrayPacket3.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found array size", arrayPacket3.getDecodeErr());

    ArrayPacket arrayPacket4;
    buf.clear();
    buf.append("*123abc\r\n");
    ASSERT_EQ(DState::ERROR, arrayPacket4.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: integer format error", arrayPacket4.getDecodeErr());

    ArrayPacket arrayPacket5;
    buf.clear();
    buf.append("*-2\r\n");
    ASSERT_EQ(DState::SUCCESS, arrayPacket5.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(PacketType::TYPE_NULL, arrayPacket5.getType());
    ASSERT_EQ(5, arrayPacket5.getRESP2EncodeSize());

    ArrayPacket arrayPacket6;
    buf.clear();
    buf.append("*-1\r\n");
    ASSERT_EQ(DState::SUCCESS, arrayPacket6.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(PacketType::TYPE_NULL, arrayPacket6.getType());
    ASSERT_EQ(5, arrayPacket6.getRESP2EncodeSize());

    ArrayPacket arrayPacket7;
    buf.clear();
    buf.append("*1\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket7.decodeRESP3(&buf));
    buf.append("-test\r\n");
    ASSERT_EQ(DState::SUCCESS, arrayPacket7.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(11, arrayPacket7.getRESP2EncodeSize());

    ArrayPacket arrayPacket8;
    buf.clear();
    buf.append("*1\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket8.decodeRESP3(&buf));
    buf.append("&test\r\n");
    ASSERT_EQ(DState::ERROR, arrayPacket8.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: unknown packet type: '&'", arrayPacket8.getDecodeErr());

    ArrayPacket arrayPacket9;
    buf.clear();
    buf.append("*1\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket9.decodeRESP3(&buf));
    buf.append(":\r\n");
    ASSERT_EQ(DState::ERROR, arrayPacket9.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found integer", arrayPacket9.getDecodeErr());

    ArrayPacket arrayPacket10;
    buf.clear();
    buf.append("*");
    ASSERT_EQ(DState::AGAIN, arrayPacket10.decodeRESP3(&buf));
    buf.append("2\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket10.decodeRESP3(&buf));
    buf.append(":");
    ASSERT_EQ(DState::AGAIN, arrayPacket10.decodeRESP3(&buf));
    buf.append("10\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket10.decodeRESP3(&buf));
    buf.append("+");
    ASSERT_EQ(DState::AGAIN, arrayPacket10.decodeRESP3(&buf));
    buf.append("OK\r\n");
    ASSERT_EQ(DState::SUCCESS, arrayPacket10.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(14, arrayPacket10.getRESP2EncodeSize());
    auto packets = arrayPacket10.getPacketArray();
    ASSERT_EQ(2U, packets.size());
    ASSERT_TRUE(packets[0]->instance_of<IntegerPacket>());
    ASSERT_EQ(10, packets[0]->packet_cast<IntegerPacket>()->getValue());
    ASSERT_TRUE(packets[1]->instance_of<SimpleStringPacket>());
    ASSERT_EQ("OK", packets[1]->packet_cast<SimpleStringPacket>()->getValue());

    ArrayPacket arrayPacket11;
    buf.clear();
    buf.append("*");
    ASSERT_EQ(DState::AGAIN, arrayPacket11.decodeRESP3(&buf));
    buf.append("2\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket11.decodeRESP3(&buf));
    buf.append(":");
    ASSERT_EQ(DState::AGAIN, arrayPacket11.decodeRESP3(&buf));
    buf.append("10\r\n");
    ASSERT_EQ(DState::AGAIN, arrayPacket11.decodeRESP3(&buf));
    buf.append(",");
    ASSERT_EQ(DState::AGAIN, arrayPacket11.decodeRESP3(&buf));
    buf.append("1.2\r\n");
    ASSERT_EQ(DState::SUCCESS, arrayPacket11.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(15, arrayPacket11.getRESP3EncodeSize());
    packets = arrayPacket11.getPacketArray();
    ASSERT_EQ(2U, packets.size());
    ASSERT_TRUE(packets[0]->instance_of<IntegerPacket>());
    ASSERT_EQ(10, packets[0]->packet_cast<IntegerPacket>()->getValue());
    ASSERT_TRUE(packets[1]->instance_of<DoublePacket>());
    ASSERT_DOUBLE_EQ(1.2, packets[1]->packet_cast<DoublePacket>()->getValue());
}
