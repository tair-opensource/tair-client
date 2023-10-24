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
#include "protocol/packet/resp/NullPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::NullPacket;
using tair::protocol::NullType;

TEST(NULL_PACKET_TEST, ENCODE_TEST) {
    NullPacket null(NullType::null_bulk);
    Buffer buf;
    null.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), null.getRESP2EncodeSize());
    ASSERT_EQ("$-1\r\n", buf.nextAllString());

    NullPacket null2(NullType::null_multi_bulk);
    buf.clear();
    null2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), null2.getRESP2EncodeSize());
    ASSERT_EQ("*-1\r\n", buf.nextAllString());

    NullPacket null3;
    buf.clear();
    null3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), null3.getRESP3EncodeSize());
    ASSERT_EQ("_\r\n", buf.nextAllString());
}

TEST(NULL_PACKET_TEST, DECODE_TEST) {
    NullPacket null;
    Buffer buf;
    ASSERT_EQ(DState::ERROR, null.decodeRESP2(&buf));

    ASSERT_EQ(DState::AGAIN, null.decodeRESP3(&buf));
    buf.append("_");
    ASSERT_EQ(DState::AGAIN, null.decodeRESP3(&buf));
    buf.append("\r\n");
    ASSERT_EQ(DState::SUCCESS, null.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());

    buf.clear();
    buf.append("?\r\r");
    ASSERT_EQ(DState::ERROR, null.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '_', got '?'", null.getDecodeErr());

    buf.clear();
    buf.append("_\r\r");
    ASSERT_EQ(DState::ERROR, null.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected <CR><LF>, got '\r''\r'", null.getDecodeErr());

    buf.clear();
    buf.append("_\r\n_\r\n");
    ASSERT_EQ(DState::SUCCESS, null.decodeRESP3(&buf));
    ASSERT_EQ(DState::SUCCESS, null.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
}
