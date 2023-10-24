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
#include "protocol/packet/resp/DoublePacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::DoublePacket;

TEST(DOUBLE_PACKET_TEST, ENCODE_TEST) {
    DoublePacket double1(0);
    Buffer buf;
    double1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), double1.getRESP2EncodeSize());
    ASSERT_EQ("$1\r\n0\r\n", buf.nextAllString());
    buf.clear();
    double1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), double1.getRESP3EncodeSize());
    ASSERT_EQ(",0\r\n", buf.nextAllString());

    DoublePacket double2(1.2);
    double2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), double2.getRESP2EncodeSize());
    ASSERT_EQ("$3\r\n1.2\r\n", buf.nextAllString());
    buf.clear();
    double2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), double2.getRESP3EncodeSize());
    ASSERT_EQ(",1.2\r\n", buf.nextAllString());

    DoublePacket double3(1.0 / 0.0);
    double3.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), double3.getRESP2EncodeSize());
    ASSERT_EQ("$3\r\ninf\r\n", buf.nextAllString());
    buf.clear();
    double3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), double3.getRESP3EncodeSize());
    ASSERT_EQ(",inf\r\n", buf.nextAllString());

    DoublePacket double4(-1.0 / 0.0);
    double4.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), double4.getRESP2EncodeSize());
    ASSERT_EQ("$4\r\n-inf\r\n", buf.nextAllString());
    buf.clear();
    double4.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), double4.getRESP3EncodeSize());
    ASSERT_EQ(",-inf\r\n", buf.nextAllString());
}

TEST(DOUBLE_PACKET_TEST, DECODE_TEST) {
    DoublePacket number1;
    Buffer buf;
    ASSERT_EQ(DState::ERROR, number1.decodeRESP2(&buf));

    ASSERT_EQ(DState::AGAIN, number1.decodeRESP3(&buf));
    buf.append(",");
    ASSERT_EQ(DState::AGAIN, number1.decodeRESP3(&buf));
    buf.append("\r\n");
    ASSERT_EQ(DState::ERROR, number1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found double", number1.getDecodeErr());

    buf.clear();
    buf.append("*123\r\n");
    ASSERT_EQ(DState::ERROR, number1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected ',', got '*'", number1.getDecodeErr());

    buf.clear();
    buf.append(",aaa\r\n");
    ASSERT_EQ(DState::ERROR, number1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: double format error", number1.getDecodeErr());

    buf.clear();
    buf.append(",123\r\n");
    ASSERT_EQ(DState::SUCCESS, number1.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_DOUBLE_EQ(123, number1.getValue());

    buf.clear();
    buf.append(",-1234\r\n");
    ASSERT_EQ(DState::SUCCESS, number1.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_DOUBLE_EQ(-1234, number1.getValue());
    ASSERT_EQ(8, number1.getRESP3EncodeSize());
    ASSERT_EQ(11, number1.getRESP2EncodeSize());

    buf.clear();
    buf.append(",0\r\n");
    ASSERT_EQ(DState::SUCCESS, number1.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_DOUBLE_EQ(0, number1.getValue());
    ASSERT_EQ(4, number1.getRESP3EncodeSize());
    ASSERT_EQ(7, number1.getRESP2EncodeSize());

    buf.clear();
    buf.append(",1.2\r\n,1.2\r\n");
    ASSERT_EQ(DState::SUCCESS, number1.decodeRESP3(&buf));
    ASSERT_DOUBLE_EQ(1.2, number1.getValue());
    ASSERT_EQ(6, number1.getRESP3EncodeSize());
    ASSERT_EQ(9, number1.getRESP2EncodeSize());
    ASSERT_EQ(DState::SUCCESS, number1.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_DOUBLE_EQ(1.2, number1.getValue());
    ASSERT_EQ(6, number1.getRESP3EncodeSize());
    ASSERT_EQ(9, number1.getRESP2EncodeSize());
    ASSERT_EQ(0, buf.size());
}
