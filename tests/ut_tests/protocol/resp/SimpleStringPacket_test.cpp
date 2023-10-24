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
#include "protocol/packet/resp/SimpleStringPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::SimpleStringPacket;

TEST(SIMPLE_STRING_PACKET_TEST, ENCODE_TEST) {
    SimpleStringPacket simpleString1("simple");
    Buffer buf;
    simpleString1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), simpleString1.getRESP2EncodeSize());
    ASSERT_EQ("+simple\r\n", buf.nextAllString());
    buf.clear();
    simpleString1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), simpleString1.getRESP3EncodeSize());
    ASSERT_EQ("+simple\r\n", buf.nextAllString());

    SimpleStringPacket simpleString2("");
    buf.clear();
    simpleString2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), simpleString2.getRESP2EncodeSize());
    ASSERT_EQ("+\r\n", buf.nextAllString());
    buf.clear();
    simpleString2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), simpleString2.getRESP3EncodeSize());
    ASSERT_EQ("+\r\n", buf.nextAllString());
}

TEST(SIMPLE_STRING_PACKET_TEST, DECODE_TEST) {
    SimpleStringPacket simpleString;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, simpleString.decodeRESP2(&buf));
    buf.append("*333\r\n");
    ASSERT_EQ(DState::ERROR, simpleString.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: expected '+', got '*'", simpleString.getDecodeErr());

    buf.clear();
    buf.append("+12345");
    ASSERT_EQ(DState::AGAIN, simpleString.decodeRESP2(&buf));

    buf.clear();
    buf.append("+\r\n");
    ASSERT_EQ(DState::SUCCESS, simpleString.decodeRESP2(&buf));
    ASSERT_EQ("", simpleString.getValue());
    ASSERT_EQ(3, simpleString.getRESP2EncodeSize());

    buf.clear();
    buf.append("+OK\r\n");
    ASSERT_EQ(DState::SUCCESS, simpleString.decodeRESP2(&buf));
    ASSERT_EQ("OK", simpleString.getValue());
    ASSERT_EQ(5, simpleString.getRESP2EncodeSize());

    buf.clear();
    buf.append("+OK Redis\r\n");
    ASSERT_EQ(DState::SUCCESS, simpleString.decodeRESP2(&buf));
    ASSERT_EQ("OK Redis", simpleString.getValue());
    ASSERT_EQ(11, simpleString.getRESP2EncodeSize());

    buf.clear();
    buf.append("+Hello Redis\r\n+Hello Tair\r\n");
    ASSERT_EQ(DState::SUCCESS, simpleString.decodeRESP2(&buf));
    ASSERT_EQ("Hello Redis", simpleString.getValue());
    ASSERT_EQ(14, simpleString.getRESP2EncodeSize());
    ASSERT_EQ(DState::SUCCESS, simpleString.decodeRESP2(&buf));
    ASSERT_EQ("Hello Tair", simpleString.getValue());
    ASSERT_EQ(13, simpleString.getRESP2EncodeSize());
}

TEST(SIMPLE_STRING_PACKET_TEST, DECODE3_TEST) {
    SimpleStringPacket simpleString;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, simpleString.decodeRESP3(&buf));
    buf.append("*333\r\n");
    ASSERT_EQ(DState::ERROR, simpleString.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '+', got '*'", simpleString.getDecodeErr());

    buf.clear();
    buf.append("+12345");
    ASSERT_EQ(DState::AGAIN, simpleString.decodeRESP3(&buf));

    buf.clear();
    buf.append("+\r\n");
    ASSERT_EQ(DState::SUCCESS, simpleString.decodeRESP3(&buf));
    ASSERT_EQ("", simpleString.getValue());
    ASSERT_EQ(3, simpleString.getRESP2EncodeSize());
    ASSERT_TRUE(buf.empty());

    buf.clear();
    buf.append("+OK\r\n");
    ASSERT_EQ(DState::SUCCESS, simpleString.decodeRESP3(&buf));
    ASSERT_EQ("OK", simpleString.getValue());
    ASSERT_EQ(5, simpleString.getRESP2EncodeSize());
    ASSERT_TRUE(buf.empty());

    buf.clear();
    buf.append("+OK Redis\r\n");
    ASSERT_EQ(DState::SUCCESS, simpleString.decodeRESP3(&buf));
    ASSERT_EQ("OK Redis", simpleString.getValue());
    ASSERT_EQ(11, simpleString.getRESP2EncodeSize());
    ASSERT_TRUE(buf.empty());

    buf.clear();
    buf.append("+Hello Redis\r\n+Hello Tair\r\n");
    ASSERT_EQ(DState::SUCCESS, simpleString.decodeRESP3(&buf));
    ASSERT_EQ("Hello Redis", simpleString.getValue());
    ASSERT_EQ(14, simpleString.getRESP2EncodeSize());
    ASSERT_EQ(DState::SUCCESS, simpleString.decodeRESP3(&buf));
    ASSERT_EQ("Hello Tair", simpleString.getValue());
    ASSERT_EQ(13, simpleString.getRESP2EncodeSize());
    ASSERT_TRUE(buf.empty());
}
