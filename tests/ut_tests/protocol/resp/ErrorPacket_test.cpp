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
#include "protocol/packet/resp/ErrorPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::ErrorPacket;

TEST(ERROR_PACKET_TEST, ENCODE_TEST) {
    ErrorPacket error1("error");
    Buffer buf;
    error1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), error1.getRESP2EncodeSize());
    ASSERT_EQ("-error\r\n", buf.nextAllString());
    buf.clear();
    error1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), error1.getRESP3EncodeSize());
    ASSERT_EQ("-error\r\n", buf.nextAllString());

    ErrorPacket error2("");
    buf.clear();
    error2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), error2.getRESP2EncodeSize());
    ASSERT_EQ("-\r\n", buf.nextAllString());
    buf.clear();
    error2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), error2.getRESP3EncodeSize());
    ASSERT_EQ("-\r\n", buf.nextAllString());
}

TEST(ERROR_PACKET_TEST, DECODE_TEST) {
    ErrorPacket error;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, error.decodeRESP2(&buf));
    buf.append("*333\r\n");
    ASSERT_EQ(DState::ERROR, error.decodeRESP2(&buf));
    ASSERT_EQ("Protocol error: expected '-', got '*'", error.getDecodeErr());

    buf.clear();
    buf.append("-12345");
    ASSERT_EQ(DState::AGAIN, error.decodeRESP2(&buf));

    buf.clear();
    buf.append("-\r\n");
    ASSERT_EQ(DState::SUCCESS, error.decodeRESP2(&buf));
    ASSERT_EQ("", error.getValue());
    ASSERT_EQ(3, error.getRESP2EncodeSize());

    buf.clear();
    buf.append("-error\r\n");
    ASSERT_EQ(DState::SUCCESS, error.decodeRESP2(&buf));
    ASSERT_EQ("error", error.getValue());
    ASSERT_EQ(8, error.getRESP2EncodeSize());

    buf.clear();
    buf.append("-error info\r\n");
    ASSERT_EQ(DState::SUCCESS, error.decodeRESP2(&buf));
    ASSERT_EQ("error info", error.getValue());
    ASSERT_EQ(13, error.getRESP2EncodeSize());

    buf.clear();
    buf.append("-error info\r\n-error info2\r\n");
    ASSERT_EQ(DState::SUCCESS, error.decodeRESP2(&buf));
    ASSERT_EQ("error info", error.getValue());
    ASSERT_EQ(13, error.getRESP2EncodeSize());
    ASSERT_EQ(DState::SUCCESS, error.decodeRESP2(&buf));
    ASSERT_EQ("error info2", error.getValue());
    ASSERT_EQ(14, error.getRESP2EncodeSize());
}

TEST(ERROR_PACKET_TEST, DECODE3_TEST) {
    ErrorPacket error;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, error.decodeRESP3(&buf));
    buf.append("*333\r\n");
    ASSERT_EQ(DState::ERROR, error.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '-', got '*'", error.getDecodeErr());

    buf.clear();
    buf.append("-12345");
    ASSERT_EQ(DState::AGAIN, error.decodeRESP3(&buf));

    buf.clear();
    buf.append("-\r\n");
    ASSERT_EQ(DState::SUCCESS, error.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("", error.getValue());
    ASSERT_EQ(3, error.getRESP2EncodeSize());

    buf.clear();
    buf.append("-error\r\n");
    ASSERT_EQ(DState::SUCCESS, error.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("error", error.getValue());
    ASSERT_EQ(8, error.getRESP2EncodeSize());

    buf.clear();
    buf.append("-error info\r\n");
    ASSERT_EQ(DState::SUCCESS, error.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("error info", error.getValue());
    ASSERT_EQ(13, error.getRESP2EncodeSize());

    buf.clear();
    buf.append("-error info\r\n-error info2\r\n");
    ASSERT_EQ(DState::SUCCESS, error.decodeRESP3(&buf));
    ASSERT_EQ("error info", error.getValue());
    ASSERT_EQ(13, error.getRESP2EncodeSize());
    ASSERT_EQ(DState::SUCCESS, error.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("error info2", error.getValue());
    ASSERT_EQ(14, error.getRESP2EncodeSize());
}
