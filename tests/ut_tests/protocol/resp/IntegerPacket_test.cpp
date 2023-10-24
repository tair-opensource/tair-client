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
#include "protocol/packet/resp/IntegerPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::IntegerPacket;

TEST(INTEGER_PACKET_TEST, ENCODE_TEST) {
    IntegerPacket integer1(10);
    Buffer buf;
    integer1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), integer1.getRESP2EncodeSize());
    ASSERT_EQ(":10\r\n", buf.nextAllString());
    buf.clear();
    integer1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), integer1.getRESP3EncodeSize());
    ASSERT_EQ(":10\r\n", buf.nextAllString());

    IntegerPacket integer2(0);
    buf.clear();
    integer2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), integer2.getRESP2EncodeSize());
    ASSERT_EQ(":0\r\n", buf.nextAllString());
    buf.clear();
    integer2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), integer2.getRESP3EncodeSize());
    ASSERT_EQ(":0\r\n", buf.nextAllString());

    IntegerPacket integer3(-10);
    buf.clear();
    integer3.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), integer3.getRESP2EncodeSize());
    ASSERT_EQ(":-10\r\n", buf.nextAllString());
    buf.clear();
    integer3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), integer3.getRESP3EncodeSize());
    ASSERT_EQ(":-10\r\n", buf.nextAllString());
}

TEST(INTEGER_PACKET_TEST, DECODE_TEST) {
    IntegerPacket integer;
    ASSERT_EQ(0, integer.getValue());
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, integer.decodeRESP2(&buf));
    buf.append(":");
    ASSERT_EQ(DState::AGAIN, integer.decodeRESP2(&buf));
    buf.append("\r\n");
    ASSERT_EQ(DState::ERROR, integer.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: not found integer", integer.getDecodeErr());

    buf.clear();
    buf.append("*123\r\n");
    ASSERT_EQ(DState::ERROR, integer.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: expected ':', got '*'", integer.getDecodeErr());

    buf.clear();
    buf.append(":123abc\r\n");
    ASSERT_EQ(DState::ERROR, integer.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: integer format error", integer.getDecodeErr());

    buf.clear();
    buf.append(":23\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP2(&buf));
    ASSERT_EQ(23, integer.getValue());
    ASSERT_TRUE(buf.empty());

    buf.clear();
    buf.append(":-1234\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP2(&buf));
    ASSERT_EQ(-1234, integer.getValue());
    ASSERT_EQ(8, integer.getRESP2EncodeSize());

    buf.clear();
    buf.append(":0\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP2(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(0, integer.getValue());
    ASSERT_EQ(4, integer.getRESP2EncodeSize());

    buf.clear();
    buf.append(":0\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP2(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(0, integer.getValue());
    ASSERT_EQ(4, integer.getRESP2EncodeSize());

    buf.clear();
    buf.append(":123\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP2(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(123, integer.getValue());
    ASSERT_EQ(6, integer.getRESP2EncodeSize());

    buf.clear();
    buf.append(":675\r\n:-12\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP2(&buf));
    ASSERT_EQ(675, integer.getValue());
    ASSERT_EQ(6, integer.getRESP2EncodeSize());
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP2(&buf));
    ASSERT_EQ(-12, integer.getValue());
    ASSERT_EQ(6, integer.getRESP2EncodeSize());
    ASSERT_EQ(0, buf.size());
}

TEST(INTEGER_PACKET_TEST, DECODE3_TEST) {
    IntegerPacket integer;
    ASSERT_EQ(0, integer.getValue());
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, integer.decodeRESP3(&buf));
    buf.append(":");
    ASSERT_EQ(DState::AGAIN, integer.decodeRESP3(&buf));
    buf.append("\r\n");
    ASSERT_EQ(DState::ERROR, integer.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found integer", integer.getDecodeErr());

    buf.clear();
    buf.append("*123\r\n");
    ASSERT_EQ(DState::ERROR, integer.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected ':', got '*'", integer.getDecodeErr());

    buf.clear();
    buf.append(":123abc\r\n");
    ASSERT_EQ(DState::ERROR, integer.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: integer format error", integer.getDecodeErr());

    buf.clear();
    buf.append(":23\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(23, integer.getValue());

    buf.clear();
    buf.append(":-1234\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(-1234, integer.getValue());
    ASSERT_EQ(8, integer.getRESP2EncodeSize());

    buf.clear();
    buf.append(":0\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(0, integer.getValue());
    ASSERT_EQ(4, integer.getRESP2EncodeSize());

    buf.clear();
    buf.append(":123\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(123, integer.getValue());
    ASSERT_EQ(6, integer.getRESP2EncodeSize());

    buf.clear();
    buf.append(":675\r\n:-12\r\n");
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP3(&buf));
    ASSERT_EQ(675, integer.getValue());
    ASSERT_EQ(6, integer.getRESP2EncodeSize());
    ASSERT_EQ(DState::SUCCESS, integer.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(-12, integer.getValue());
    ASSERT_EQ(6, integer.getRESP2EncodeSize());
    ASSERT_EQ(0, buf.size());
}
