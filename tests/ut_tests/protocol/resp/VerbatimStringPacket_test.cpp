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
#include "protocol/packet/resp/VerbatimStringPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::VerbatimStringPacket;

TEST(VERBATIM_STRING_PACKET_TEST, ENCODE_TEST) {
    VerbatimStringPacket ver1("format", "txt");
    Buffer buf;
    ver1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), ver1.getRESP3EncodeSize());
    ASSERT_EQ("=10\r\ntxt:format\r\n", buf.nextAllString());
    buf.clear();
    ver1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), ver1.getRESP2EncodeSize());
    ASSERT_EQ("$6\r\nformat\r\n", buf.nextAllString());

    VerbatimStringPacket ver2("", "txt");
    buf.clear();
    ver2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), ver2.getRESP3EncodeSize());
    ASSERT_EQ("=4\r\ntxt:\r\n", buf.nextAllString());
    buf.clear();
    ver2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), ver2.getRESP2EncodeSize());
    ASSERT_EQ("$0\r\n\r\n", buf.nextAllString());

    VerbatimStringPacket ver3("test\r\n", "txt");
    buf.clear();
    ver3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), ver3.getRESP3EncodeSize());
    ASSERT_EQ("=10\r\ntxt:test\r\n\r\n", buf.nextAllString());
    buf.clear();
    ver3.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), ver3.getRESP2EncodeSize());
    ASSERT_EQ("$6\r\ntest\r\n\r\n", buf.nextAllString());

    VerbatimStringPacket ver4("test\r\n", "txt123");
    buf.clear();
    ver4.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), ver4.getRESP3EncodeSize());
    ASSERT_EQ("=10\r\ntxt:test\r\n\r\n", buf.nextAllString());
    buf.clear();
    ver4.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), ver4.getRESP2EncodeSize());
    ASSERT_EQ("$6\r\ntest\r\n\r\n", buf.nextAllString());

    VerbatimStringPacket ver5("test\r\n", " ");
    buf.clear();
    ver5.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), ver5.getRESP3EncodeSize());
    ASSERT_EQ("=10\r\n   :test\r\n\r\n", buf.nextAllString());
    buf.clear();
    ver5.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), ver5.getRESP2EncodeSize());
    ASSERT_EQ("$6\r\ntest\r\n\r\n", buf.nextAllString());
}

TEST(VERBATIM_STRING_PACKET_TEST, DECODE_TEST) {
    VerbatimStringPacket ver1;
    Buffer buf;
    ASSERT_EQ(DState::ERROR, ver1.decodeRESP2(&buf));
    ASSERT_EQ(DState::AGAIN, ver1.decodeRESP3(&buf));
    buf.append("*333\r\n");
    ASSERT_EQ(DState::ERROR, ver1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '=', got '*'", ver1.getDecodeErr());

    VerbatimStringPacket ver2;
    buf.clear();
    buf.append("=");
    buf.append(std::string(1024 * 65, 'a'));
    ASSERT_EQ(DState::ERROR, ver2.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: too big bulk count string", ver2.getDecodeErr());

    VerbatimStringPacket ver3;
    buf.clear();
    buf.append("=\r\n");
    ASSERT_EQ(DState::ERROR, ver3.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found bulkstring len", ver3.getDecodeErr());

    VerbatimStringPacket ver4;
    buf.clear();
    buf.append("=abc\r\n");
    ASSERT_EQ(DState::ERROR, ver4.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: invalid bulk length", ver4.getDecodeErr());

    VerbatimStringPacket ver5;
    buf.clear();
    buf.append("=-2\r\n");
    ASSERT_EQ(DState::ERROR, ver5.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: invalid bulk length", ver5.getDecodeErr());

    VerbatimStringPacket ver6;
    buf.clear();
    buf.append("=2");
    ASSERT_EQ(DState::AGAIN, ver6.decodeRESP3(&buf));

    VerbatimStringPacket ver7;
    buf.clear();
    buf.append("=1448576\r\n");
    ASSERT_EQ(DState::AGAIN, ver7.decodeRESP3(&buf));

    VerbatimStringPacket ver8;
    buf.clear();
    buf.append("=13\r\ntxt:hello%d\r\n\r\n");
    ASSERT_EQ(DState::SUCCESS, ver8.decodeRESP3(&buf));
    ASSERT_EQ("hello%d\r\n", ver8.getValue());
    ASSERT_EQ("txt", ver8.getExt());
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(20, ver8.getRESP3EncodeSize());

    VerbatimStringPacket ver9;
    buf.clear();
    buf.append("=-1\r\n$0\r\n\r\n$10\r\nREDISREDIS\r\n");
    ASSERT_EQ(DState::ERROR, ver9.decodeRESP3(&buf));
    ASSERT_EQ("", ver9.getValue());

    VerbatimStringPacket ver10;
    buf.clear();
    buf.append("=10\r\ntxt:test\r\n\r\n=12\r\ntxt:\r\ntest\r\n\r\n");
    ASSERT_EQ(DState::SUCCESS, ver10.decodeRESP3(&buf));
    ASSERT_EQ("test\r\n", ver10.getValue());
    ASSERT_EQ("txt", ver10.getExt());
    ASSERT_EQ(17, ver10.getRESP3EncodeSize());
    ASSERT_EQ(DState::SUCCESS, ver10.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("\r\ntest\r\n", ver10.getValue());
    ASSERT_EQ("txt", ver10.getExt());
    ASSERT_EQ(19, ver10.getRESP3EncodeSize());
}
