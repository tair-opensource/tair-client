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
#include "protocol/packet/resp/BulkStringPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::PacketType;
using tair::protocol::BulkStringPacket;

TEST(BULK_STRING_PACKET_TEST, ENCODE_TEST) {
    BulkStringPacket bulkString1("bulk");
    Buffer buf;
    bulkString1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), bulkString1.getRESP2EncodeSize());
    ASSERT_EQ("$4\r\nbulk\r\n", buf.nextAllString());
    buf.clear();
    bulkString1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), bulkString1.getRESP3EncodeSize());
    ASSERT_EQ("$4\r\nbulk\r\n", buf.nextAllString());

    const std::string empty_str;
    BulkStringPacket bulkString2(&empty_str);
    buf.clear();
    bulkString2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), bulkString2.getRESP2EncodeSize());
    ASSERT_EQ("$0\r\n\r\n", buf.nextAllString());
    buf.clear();
    bulkString2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), bulkString2.getRESP3EncodeSize());
    ASSERT_EQ("$0\r\n\r\n", buf.nextAllString());

    BulkStringPacket bulkString3(PacketType::TYPE_NULL);
    buf.clear();
    bulkString3.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), bulkString3.getRESP2EncodeSize());
    ASSERT_EQ("$-1\r\n", buf.nextAllString());
    buf.clear();
    bulkString3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), bulkString3.getRESP3EncodeSize());
    ASSERT_EQ("$-1\r\n", buf.nextAllString());
}

TEST(BULK_STRING_PACKET_TEST, DECODE_TEST) {
    BulkStringPacket bulkString1;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, bulkString1.decodeRESP2(&buf));
    buf.append("*333\r\n");
    ASSERT_EQ(DState::ERROR, bulkString1.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: expected '$', got '*'", bulkString1.getDecodeErr());

    BulkStringPacket bulkString2;
    buf.clear();
    buf.append("$");
    buf.append(std::string(1024 * 65, 'a'));
    ASSERT_EQ(DState::ERROR, bulkString2.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: too big bulk count string", bulkString2.getDecodeErr());

    BulkStringPacket bulkString3;
    buf.clear();
    buf.append("$\r\n");
    ASSERT_EQ(DState::ERROR, bulkString3.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: not found bulkstring len", bulkString3.getDecodeErr());

    BulkStringPacket bulkString4;
    buf.clear();
    buf.append("$abc\r\n");
    ASSERT_EQ(DState::ERROR, bulkString4.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: invalid bulk length", bulkString4.getDecodeErr());

    BulkStringPacket bulkString5;
    buf.clear();
    buf.append("$-2\r\n");
    ASSERT_EQ(DState::ERROR, bulkString5.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: invalid bulk length", bulkString5.getDecodeErr());

    BulkStringPacket bulkString6;
    buf.clear();
    buf.append("$2");
    ASSERT_EQ(DState::AGAIN, bulkString6.decodeRESP2(&buf));

    BulkStringPacket bulkString7;
    buf.clear();
    buf.append("$1448576\r\n");
    ASSERT_EQ(DState::AGAIN, bulkString7.decodeRESP2(&buf));

    BulkStringPacket bulkString8;
    buf.clear();
    buf.append("$5\r\nhello\r\n");
    ASSERT_EQ(DState::SUCCESS, bulkString8.decodeRESP2(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("hello", bulkString8.getValue());
    ASSERT_EQ(PacketType::TYPE_COMMON, bulkString8.getType());
    ASSERT_EQ(11, bulkString8.getRESP2EncodeSize());

    BulkStringPacket bulkString9;
    buf.clear();
    buf.append("$-1\r\n$0\r\n\r\n$10\r\nREDISREDIS\r\n");
    ASSERT_EQ(DState::SUCCESS, bulkString9.decodeRESP2(&buf));
    ASSERT_EQ(PacketType::TYPE_NULL, bulkString9.getType());
    ASSERT_EQ("", bulkString9.getValue());
    ASSERT_EQ(5, bulkString9.getRESP2EncodeSize());

    BulkStringPacket bulkString10;
    ASSERT_EQ(DState::SUCCESS, bulkString10.decodeRESP2(&buf));
    ASSERT_EQ(PacketType::TYPE_COMMON, bulkString10.getType());
    ASSERT_EQ("", bulkString10.getValue());
    ASSERT_EQ(6, bulkString10.getRESP2EncodeSize());
    ASSERT_EQ(DState::SUCCESS, bulkString10.decodeRESP2(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(PacketType::TYPE_COMMON, bulkString10.getType());
    ASSERT_EQ("REDISREDIS", bulkString10.getValue());
    ASSERT_EQ(17, bulkString10.getRESP2EncodeSize());
}

TEST(BULK_STRING_PACKET_TEST, DECODE3_TEST) {
    BulkStringPacket bulkString1;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, bulkString1.decodeRESP3(&buf));
    buf.append("*333\r\n");
    ASSERT_EQ(DState::ERROR, bulkString1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '$', got '*'", bulkString1.getDecodeErr());

    BulkStringPacket bulkString2;
    buf.clear();
    buf.append("$");
    buf.append(std::string(1024 * 65, 'a'));
    ASSERT_EQ(DState::ERROR, bulkString2.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: too big bulk count string", bulkString2.getDecodeErr());

    BulkStringPacket bulkString3;
    buf.clear();
    buf.append("$\r\n");
    ASSERT_EQ(DState::ERROR, bulkString3.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found bulkstring len", bulkString3.getDecodeErr());

    BulkStringPacket bulkString4;
    buf.clear();
    buf.append("$abc\r\n");
    ASSERT_EQ(DState::ERROR, bulkString4.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: invalid bulk length", bulkString4.getDecodeErr());

    BulkStringPacket bulkString5;
    buf.clear();
    buf.append("$-2\r\n");
    ASSERT_EQ(DState::ERROR, bulkString5.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: invalid bulk length", bulkString5.getDecodeErr());

    BulkStringPacket bulkString6;
    buf.clear();
    buf.append("$2");
    ASSERT_EQ(DState::AGAIN, bulkString6.decodeRESP3(&buf));

    BulkStringPacket bulkString7;
    buf.clear();
    buf.append("$1448576\r\n");
    ASSERT_EQ(DState::AGAIN, bulkString7.decodeRESP3(&buf));

    BulkStringPacket bulkString8;
    buf.clear();
    buf.append("$5\r\nhello\r\n");
    ASSERT_EQ(DState::SUCCESS, bulkString8.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("hello", bulkString8.getValue());
    ASSERT_EQ(PacketType::TYPE_COMMON, bulkString8.getType());
    ASSERT_EQ(11, bulkString8.getRESP2EncodeSize());

    BulkStringPacket bulkString9;
    buf.clear();
    buf.append("$-1\r\n$0\r\n\r\n$10\r\nREDISREDIS\r\n");
    ASSERT_EQ(DState::SUCCESS, bulkString9.decodeRESP3(&buf));
    ASSERT_EQ(PacketType::TYPE_NULL, bulkString9.getType());
    ASSERT_EQ("", bulkString9.getValue());
    ASSERT_EQ(5, bulkString9.getRESP2EncodeSize());

    BulkStringPacket bulkString10;
    ASSERT_EQ(DState::SUCCESS, bulkString10.decodeRESP3(&buf));
    ASSERT_EQ(PacketType::TYPE_COMMON, bulkString10.getType());
    ASSERT_EQ("", bulkString10.getValue());
    ASSERT_EQ(6, bulkString10.getRESP2EncodeSize());
    ASSERT_EQ(DState::SUCCESS, bulkString10.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(PacketType::TYPE_COMMON, bulkString10.getType());
    ASSERT_EQ("REDISREDIS", bulkString10.getValue());
    ASSERT_EQ(17, bulkString10.getRESP2EncodeSize());
}
