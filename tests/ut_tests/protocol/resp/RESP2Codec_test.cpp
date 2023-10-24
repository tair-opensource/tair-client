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
#include "protocol/codec/CodecFactory.hpp"
#include "protocol/codec/resp/RESP2Codec.hpp"
#include "protocol/packet/resp/ArrayPacket.hpp"
#include "protocol/packet/resp/MapPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::Codec;
using tair::protocol::CodecType;
using tair::protocol::CodecFactory;
using tair::protocol::RESP2Codec;
using tair::protocol::PacketUniqPtr;
using tair::protocol::ArrayPacket;
using tair::protocol::BulkStringPacket;
using tair::protocol::SimpleStringPacket;
using tair::protocol::MapPacket;

TEST(RESP2_CODEC_TEST, CODEC_VERSION_TEST) {
    auto codec = CodecFactory::getCodec(CodecType::RESP2);
    ASSERT_EQ(CodecType::RESP2, codec->getCodecType());
}

TEST(RESP2_CODEC_TEST, RESP2_ENCODE_TEST) {
    RESP2Codec codec;
    ASSERT_EQ(CodecType::RESP2, codec.getCodecType());
    Buffer buf;
    ArrayPacket *array = new ArrayPacket;
    array->addReplyBulk("bulk");
    ArrayPacket *sub_array = array->startNewArray();
    sub_array->addReplyBulk("subbulk");
    sub_array->addReplyStatus("status");
    codec.encodeRequest(&buf, array);
    ASSERT_EQ(buf.size(), array->getRESP2EncodeSize());
    ASSERT_EQ("*2\r\n$4\r\nbulk\r\n*2\r\n$7\r\nsubbulk\r\n+status\r\n", buf.nextAllString());

    MapPacket map;
    map.addPacketPair(new BulkStringPacket("key"), array);
    buf.clear();
    codec.encodeResponse(&buf, &map);
    ASSERT_EQ(buf.size(), map.getRESP2EncodeSize());
    ASSERT_EQ("*2\r\n$3\r\nkey\r\n*2\r\n$4\r\nbulk\r\n*2\r\n$7\r\nsubbulk\r\n+status\r\n", buf.nextAllString());
}

TEST(RESP2_CODEC_TEST, RESP2_DECODE_TEST) {
    RESP2Codec codec;
    Buffer buf;
    buf.append("*2\r\n$4\r\nbulk\r\n*2\r\n$7\r\n");
    size_t buf_size = buf.size();
    PacketUniqPtr packet;
    ASSERT_EQ(DState::AGAIN, codec.decodeResponse(&buf, packet));

    buf.append("subbulk\r\n+status\r\n");
    buf_size += buf.size();
    ASSERT_EQ(DState::SUCCESS, codec.decodeResponse(&buf, packet));

    ASSERT_EQ(buf_size, packet->getPacketSize());
    ASSERT_EQ(buf_size, packet->getRESP2EncodeSize());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    ArrayPacket *array = packet->packet_cast<ArrayPacket>();
    auto packet_array = array->getPacketArray();
    ASSERT_TRUE(packet_array[0]->instance_of<BulkStringPacket>());
    ASSERT_EQ("bulk", packet_array[0]->packet_cast<BulkStringPacket>()->getValue());
    ASSERT_TRUE(packet_array[1]->instance_of<ArrayPacket>());
    ArrayPacket *sub_array = packet_array[1]->packet_cast<ArrayPacket>();
    auto sub_packet_array = sub_array->getPacketArray();
    ASSERT_TRUE(sub_packet_array[0]->instance_of<BulkStringPacket>());
    ASSERT_EQ("subbulk", sub_packet_array[0]->packet_cast<BulkStringPacket>()->getValue());
    ASSERT_TRUE(sub_packet_array[1]->instance_of<SimpleStringPacket>());
    ASSERT_EQ("status", sub_packet_array[1]->packet_cast<SimpleStringPacket>()->getValue());

    buf.clear();
    buf.append("*2\r\n$3\r\nkey\r\n*2\r\n$4\r\n");
    buf_size = buf.size();

    PacketUniqPtr packet2;
    ASSERT_EQ(DState::AGAIN, codec.decodeResponse(&buf, packet2));

    buf.append("bulk\r\n*2\r\n$7\r\nsubbulk\r\n+status\r\n");
    buf_size += buf.size();
    ASSERT_EQ(DState::SUCCESS, codec.decodeResponse(&buf, packet2));

    ASSERT_EQ(buf_size, packet2->getPacketSize());
    ASSERT_EQ(buf_size, packet2->getRESP2EncodeSize());
    ASSERT_TRUE(packet2->instance_of<ArrayPacket>());
    ArrayPacket *array_packet = packet2->packet_cast<ArrayPacket>();
    auto packet_array2 = array_packet->getPacketArray();
    ASSERT_EQ(2, packet_array2.size());
    ASSERT_TRUE(packet_array2[0]->instance_of<BulkStringPacket>());
    ASSERT_EQ("key", packet_array2[0]->packet_cast<BulkStringPacket>()->getValue());
    ASSERT_TRUE(packet_array2[1]->instance_of<ArrayPacket>());
    ArrayPacket *array2 = packet_array2[1]->packet_cast<ArrayPacket>();
    auto sub_packet_array2 = array2->getPacketArray();
    ASSERT_TRUE(sub_packet_array2[0]->instance_of<BulkStringPacket>());
    ASSERT_EQ("bulk", sub_packet_array2[0]->packet_cast<BulkStringPacket>()->getValue());
    ASSERT_TRUE(sub_packet_array2[1]->instance_of<ArrayPacket>());
    ArrayPacket *sub_array2 = sub_packet_array2[1]->packet_cast<ArrayPacket>();
    auto sub_sub_packet_array2 = sub_array2->getPacketArray();
    ASSERT_TRUE(sub_sub_packet_array2[0]->instance_of<BulkStringPacket>());
    ASSERT_EQ("subbulk", sub_sub_packet_array2[0]->packet_cast<BulkStringPacket>()->getValue());
    ASSERT_TRUE(sub_sub_packet_array2[1]->instance_of<SimpleStringPacket>());
    ASSERT_EQ("status", sub_sub_packet_array2[1]->packet_cast<SimpleStringPacket>()->getValue());
}

TEST(REDIS_CODEC_TEST, INLINE_DECODE_TEST) {
    PacketUniqPtr packet;

    RESP2Codec codec1;
    Buffer buf;
    buf.append("*");
    ASSERT_EQ(DState::AGAIN, codec1.decodeRequest(&buf, packet));

    RESP2Codec codec2;
    buf.clear();
    buf.append(std::string(64 * 1025, 'a'));
    ASSERT_EQ(DState::ERROR, codec2.decodeRequest(&buf, packet));
    ASSERT_EQ("Protocol error: too big inline request", codec2.getErr());

    RESP2Codec codec3;
    buf.clear();
    buf.append("set \"key value\n");
    ASSERT_EQ(DState::ERROR, codec3.decodeRequest(&buf, packet));
    ASSERT_EQ("Protocol error: unbalanced quotes in request", codec3.getErr());

    RESP2Codec codec4;
    buf.clear();
    buf.append("set key value\n");
    ASSERT_EQ(DState::SUCCESS, codec4.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());

    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ("set", argv[0]);
    ASSERT_EQ("key", argv[1]);
    ASSERT_EQ("value", argv[2]);

    RESP2Codec codec5;
    buf.clear();
    buf.append("set key value\r\n");
    ASSERT_EQ(DState::SUCCESS, codec5.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ("set", argv[0]);
    ASSERT_EQ("key", argv[1]);
    ASSERT_EQ("value", argv[2]);

    RESP2Codec codec6;
    buf.clear();
    buf.append("set key");
    auto len = buf.length();
    ASSERT_EQ(DState::AGAIN, codec6.decodeRequest(&buf, packet));
    ASSERT_EQ(len, buf.length());
    buf.append(" value\n");
    ASSERT_EQ(DState::SUCCESS, codec6.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ("set", argv[0]);
    ASSERT_EQ("key", argv[1]);
    ASSERT_EQ("value", argv[2]);
}

TEST(REDIS_CODEC_TEST, MULTI_DECODE_REQ_TEST) {
    PacketUniqPtr packet;

    RESP2Codec codec1;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, codec1.decodeRequest(&buf, packet));

    RESP2Codec codec2;
    buf.clear();
    buf.append("*");
    buf.append(std::string(1024 * 1025, 'a'));
    ASSERT_EQ(DState::ERROR, codec2.decodeRequest(&buf, packet));
    ASSERT_EQ("Protocol error: too big count string", codec2.getErr());

    RESP2Codec codec3;
    buf.clear();
    buf.append("*2147483648\r\n");
    ASSERT_EQ(DState::ERROR, codec3.decodeRequest(&buf, packet));
    ASSERT_EQ("Protocol error: invalid multibulk length", codec3.getErr());

    RESP2Codec codec4;
    buf.clear();
    buf.append("*1\r\n$1048577");
    buf.append(std::string(1024 * 1025, 'a'));
    ASSERT_EQ(DState::ERROR, codec4.decodeRequest(&buf, packet));
    ASSERT_EQ("Protocol error: too big bulk count string", codec4.getErr());

    RESP2Codec codec5;
    buf.clear();
    buf.append("*3\r\nabc\r\n");
    ASSERT_EQ(DState::ERROR, codec5.decodeRequest(&buf, packet));
    ASSERT_EQ("Protocol error: unknown packet type: 'a'", codec5.getErr());

    RESP2Codec codec6;
    buf.clear();
    buf.append("*3\r\n$536870913\r\n");
    ASSERT_EQ(DState::ERROR, codec6.decodeRequest(&buf, packet));
    ASSERT_EQ("Protocol error: invalid bulk length", codec6.getErr());

    RESP2Codec codec7;
    buf.clear();
    buf.append("*-1\r\n");
    ASSERT_EQ(DState::SUCCESS, codec7.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());

    RESP2Codec codec8;
    buf.clear();
    buf.append("*0\r\n");
    ASSERT_EQ(DState::SUCCESS, codec8.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());

    buf.clear();
    buf.append("*3\r\n$3\r\nset\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");
    ASSERT_EQ(DState::SUCCESS, codec8.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());

    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ("set", argv[0]);
    ASSERT_EQ("key", argv[1]);
    ASSERT_EQ("value", argv[2]);

    RESP2Codec codec9;
    buf.clear();
    buf.append("*3\r\n$3\r\nset\r\n");
    ASSERT_EQ(DState::AGAIN, codec9.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    buf.append("$3\r\nkey\r\n$5\r\nvalue\r\n");
    ASSERT_EQ(DState::SUCCESS, codec9.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ("set", argv[0]);
    ASSERT_EQ("key", argv[1]);
    ASSERT_EQ("value", argv[2]);

    RESP2Codec codec10;
    buf.clear();
    buf.append("*3\r\n$3\r\nset\r\n$3\r\nkey\r\n$65535\r\n");
    ASSERT_EQ(DState::AGAIN, codec10.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    std::string value(65535, 'a');
    buf.append(value);
    ASSERT_EQ(DState::AGAIN, codec10.decodeRequest(&buf, packet));
    buf.append("\r\n");
    ASSERT_EQ(DState::SUCCESS, codec10.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ("set", argv[0]);
    ASSERT_EQ("key", argv[1]);
    ASSERT_EQ(value, argv[2]);
}

TEST(RESP2_CODEC_TEST, MULTI_DECODE_RESP_TEST) {
    PacketUniqPtr packet;

    RESP2Codec codec0;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, codec0.decodeResponse(&buf, packet));

    RESP2Codec codec1;
    buf.append("&");
    ASSERT_EQ(DState::ERROR, codec1.decodeResponse(&buf, packet));
    ASSERT_EQ("Protocol error: unknown type '&'", codec1.getErr());

    RESP2Codec codec2;
    buf.clear();
    buf.append("*");
    buf.append(std::string(1024 * 1025, 'a'));
    ASSERT_EQ(DState::ERROR, codec2.decodeResponse(&buf, packet));
    ASSERT_EQ("Protocol error: too big count string", codec2.getErr());

    RESP2Codec codec3;
    buf.clear();
    buf.append("*1048577\r\n");
    ASSERT_EQ(DState::AGAIN, codec3.decodeResponse(&buf, packet));

    RESP2Codec codec4;
    buf.clear();
    buf.append("*1\r\n$1048577");
    buf.append(std::string(1024 * 1025, 'a'));
    ASSERT_EQ(DState::ERROR, codec4.decodeResponse(&buf, packet));
    ASSERT_EQ("Protocol error: too big bulk count string", codec4.getErr());

    RESP2Codec codec5;
    buf.clear();
    buf.append("*3\r\nabc\r\n");
    ASSERT_EQ(DState::ERROR, codec5.decodeResponse(&buf, packet));
    ASSERT_EQ("Protocol error: unknown packet type: 'a'", codec5.getErr());

    RESP2Codec codec6;
    buf.clear();
    buf.append("*3\r\n$536870913\r\n");
    ASSERT_EQ(DState::ERROR, codec6.decodeResponse(&buf, packet));
    ASSERT_EQ("Protocol error: invalid bulk length", codec6.getErr());

    RESP2Codec codec7;
    buf.clear();
    buf.append("*-1\r\n");
    ASSERT_EQ(DState::SUCCESS, codec7.decodeResponse(&buf, packet));
    ASSERT_EQ(0U, buf.length());

    RESP2Codec codec8;
    buf.clear();
    buf.append("*0\r\n");
    ASSERT_EQ(DState::SUCCESS, codec8.decodeResponse(&buf, packet));
    ASSERT_EQ(0U, buf.length());

    buf.clear();
    buf.append("*3\r\n$3\r\nset\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");
    ASSERT_EQ(DState::SUCCESS, codec8.decodeResponse(&buf, packet));
    ASSERT_EQ(0U, buf.length());

    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ("set", argv[0]);
    ASSERT_EQ("key", argv[1]);
    ASSERT_EQ("value", argv[2]);

    RESP2Codec codec9;
    buf.clear();
    buf.append("*3\r\n$3\r\nset\r\n");
    ASSERT_EQ(DState::AGAIN, codec9.decodeResponse(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    buf.append("$3\r\nkey\r\n$5\r\nvalue\r\n");
    ASSERT_EQ(DState::SUCCESS, codec9.decodeResponse(&buf, packet));
    ASSERT_EQ(0U, buf.length());

    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ("set", argv[0]);
    ASSERT_EQ("key", argv[1]);
    ASSERT_EQ("value", argv[2]);

    RESP2Codec codec10;
    buf.clear();
    buf.append("*3\r\n$3\r\nset\r\n$3\r\nkey\r\n$65535\r\n");
    ASSERT_EQ(DState::AGAIN, codec10.decodeResponse(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    std::string value(65535, 'a');
    buf.append(value);
    ASSERT_EQ(DState::AGAIN, codec10.decodeResponse(&buf, packet));
    buf.append("\r\n");
    ASSERT_EQ(DState::SUCCESS, codec10.decodeResponse(&buf, packet));
    ASSERT_EQ(0U, buf.length());

    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ("set", argv[0]);
    ASSERT_EQ("key", argv[1]);
    ASSERT_EQ(value, argv[2]);
}
