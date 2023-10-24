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
#include "protocol/packet/resp/BigNumberPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::BigNumberPacket;

TEST(BIG_NUMBER_PACKET_TEST, ENCODE_TEST) {
    BigNumberPacket number1("123456789987654321123456789");
    Buffer buf;
    number1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), number1.getRESP2EncodeSize());
    ASSERT_EQ("$27\r\n123456789987654321123456789\r\n", buf.nextAllString());

    BigNumberPacket number2("123456789987654321123456789");
    buf.clear();
    number2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), number2.getRESP3EncodeSize());
    ASSERT_EQ("(123456789987654321123456789\r\n", buf.nextAllString());
}

TEST(BIG_NUMBER_PACKET_TEST, DECODE_TEST) {
    BigNumberPacket number;
    ASSERT_EQ("", number.getValue());
    Buffer buf;
    ASSERT_EQ(DState::ERROR, number.decodeRESP2(&buf));

    ASSERT_EQ(DState::AGAIN, number.decodeRESP3(&buf));
    buf.append("(");
    ASSERT_EQ(DState::AGAIN, number.decodeRESP3(&buf));
    buf.append("\r\n");
    ASSERT_EQ(DState::ERROR, number.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found number", number.getDecodeErr());

    buf.clear();
    buf.append("*123\r\n");
    ASSERT_EQ(DState::ERROR, number.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '(', got '*'", number.getDecodeErr());

    buf.clear();
    buf.append("(123\r\n");
    ASSERT_EQ(DState::SUCCESS, number.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("123", number.getValue());

    buf.clear();
    buf.append("(-1234\r\n");
    ASSERT_EQ(DState::SUCCESS, number.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("-1234", number.getValue());
    ASSERT_EQ(8, number.getRESP3EncodeSize());
    ASSERT_EQ(11, number.getRESP2EncodeSize());

    buf.clear();
    buf.append("(0\r\n");
    ASSERT_EQ(DState::SUCCESS, number.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("0", number.getValue());
    ASSERT_EQ(4, number.getRESP3EncodeSize());
    ASSERT_EQ(7, number.getRESP2EncodeSize());

    buf.clear();
    buf.append("(675\r\n(-12\r\n");
    ASSERT_EQ(DState::SUCCESS, number.decodeRESP3(&buf));
    ASSERT_EQ("675", number.getValue());
    ASSERT_EQ(6, number.getRESP3EncodeSize());
    ASSERT_EQ(9, number.getRESP2EncodeSize());
    ASSERT_EQ(DState::SUCCESS, number.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("-12", number.getValue());
    ASSERT_EQ(6, number.getRESP3EncodeSize());
    ASSERT_EQ(9, number.getRESP2EncodeSize());
    ASSERT_EQ(0, buf.size());
}
