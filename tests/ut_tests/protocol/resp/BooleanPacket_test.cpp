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
#include "protocol/packet/resp/BooleanPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::BooleanPacket;

TEST(BOOLEAN_PACKET_TEST, ENCODE_TEST) {
    BooleanPacket boolean1(true);
    Buffer buf;
    boolean1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), boolean1.getRESP2EncodeSize());
    ASSERT_EQ(":1\r\n", buf.nextAllString());
    buf.clear();
    boolean1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), boolean1.getRESP3EncodeSize());
    ASSERT_EQ("#t\r\n", buf.nextAllString());

    BooleanPacket boolean2(false);
    buf.clear();
    boolean2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), boolean2.getRESP2EncodeSize());
    ASSERT_EQ(":0\r\n", buf.nextAllString());
    buf.clear();
    boolean2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), boolean1.getRESP3EncodeSize());
    ASSERT_EQ("#f\r\n", buf.nextAllString());
}

TEST(BOOLEAN_PACKET_TEST, DECODE_TEST) {
    BooleanPacket boolean;
    Buffer buf;
    ASSERT_EQ(DState::ERROR, boolean.decodeRESP2(&buf));

    ASSERT_EQ(DState::AGAIN, boolean.decodeRESP3(&buf));
    buf.append("#");
    ASSERT_EQ(DState::AGAIN, boolean.decodeRESP3(&buf));
    buf.append("t\r\n");
    ASSERT_EQ(DState::SUCCESS, boolean.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(true, boolean.getValue());

    buf.clear();
    buf.append("_t\r\n");
    ASSERT_EQ(DState::ERROR, boolean.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '#', got '_'", boolean.getDecodeErr());

    buf.clear();
    buf.append("#T\r\n");
    ASSERT_EQ(DState::ERROR, boolean.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected 't' or 'f', got 'T'", boolean.getDecodeErr());

    buf.clear();
    buf.append("#t\r\r");
    ASSERT_EQ(DState::ERROR, boolean.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected <CR><LF>, got '\r''\r'", boolean.getDecodeErr());

    buf.clear();
    buf.append("#f\r\n#t\r\n");
    ASSERT_EQ(DState::SUCCESS, boolean.decodeRESP3(&buf));
    ASSERT_EQ(false, boolean.getValue());
    ASSERT_EQ(DState::SUCCESS, boolean.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(true, boolean.getValue());
}
