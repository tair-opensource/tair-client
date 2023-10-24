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
#include "protocol/packet/resp/BlobErrorPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::BlobErrorPacket;

TEST(BLOB_ERROR_PACKET_TEST, ENCODE_TEST) {
    BlobErrorPacket blob1("bulk");
    Buffer buf;
    blob1.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), blob1.getRESP3EncodeSize());
    ASSERT_EQ("!4\r\nbulk\r\n", buf.nextAllString());
    buf.clear();
    blob1.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), blob1.getRESP2EncodeSize());
    ASSERT_EQ("-bulk\r\n", buf.nextAllString());

    const std::string empty_str;
    BlobErrorPacket blob2(&empty_str);
    buf.clear();
    blob2.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), blob2.getRESP3EncodeSize());
    ASSERT_EQ("!0\r\n\r\n", buf.nextAllString());
    buf.clear();
    blob2.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), blob2.getRESP2EncodeSize());
    ASSERT_EQ("-\r\n", buf.nextAllString());

    BlobErrorPacket blob3("test\r\n");
    buf.clear();
    blob3.encodeRESP3(&buf);
    ASSERT_EQ(buf.size(), blob3.getRESP3EncodeSize());
    ASSERT_EQ("!6\r\ntest\r\n\r\n", buf.nextAllString());
    buf.clear();
    blob3.encodeRESP2(&buf);
    ASSERT_EQ(buf.size(), blob3.getRESP2EncodeSize());
    ASSERT_EQ("-test\\r\\n\r\n", buf.nextAllString());
}

TEST(BLOB_ERROR_PACKET_TEST, DECODE_TEST) {
    BlobErrorPacket blob1;
    Buffer buf;
    ASSERT_EQ(DState::ERROR, blob1.decodeRESP2(&buf));
    ASSERT_EQ(DState::AGAIN, blob1.decodeRESP3(&buf));
    buf.append("*333\r\n");
    ASSERT_EQ(DState::ERROR, blob1.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: expected '!', got '*'", blob1.getDecodeErr());

    BlobErrorPacket blob2;
    buf.clear();
    buf.append("!");
    buf.append(std::string(1024 * 65, 'a'));
    ASSERT_EQ(DState::ERROR, blob2.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: too big bulk count string", blob2.getDecodeErr());

    BlobErrorPacket blob3;
    buf.clear();
    buf.append("!\r\n");
    ASSERT_EQ(DState::ERROR, blob3.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: not found bulkstring len", blob3.getDecodeErr());

    BlobErrorPacket blob4;
    buf.clear();
    buf.append("!abc\r\n");
    ASSERT_EQ(DState::ERROR, blob4.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: invalid bulk length", blob4.getDecodeErr());

    BlobErrorPacket blob5;
    buf.clear();
    buf.append("!-2\r\n");
    ASSERT_EQ(DState::ERROR, blob5.decodeRESP3(&buf));
    ASSERT_EQ("Protocol error: invalid bulk length", blob5.getDecodeErr());

    BlobErrorPacket blob6;
    buf.clear();
    buf.append("!2");
    ASSERT_EQ(DState::AGAIN, blob6.decodeRESP3(&buf));

    BlobErrorPacket blob7;
    buf.clear();
    buf.append("!1448576\r\n");
    ASSERT_EQ(DState::AGAIN, blob7.decodeRESP3(&buf));

    BlobErrorPacket blob8;
    buf.clear();
    buf.append("!5\r\nhello\r\n");
    ASSERT_EQ(DState::SUCCESS, blob8.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("hello", blob8.getValue());
    ASSERT_EQ(11, blob8.getRESP3EncodeSize());

    BlobErrorPacket blob9;
    buf.clear();
    buf.append("!-1\r\n$0\r\n\r\n$10\r\nREDISREDIS\r\n");
    ASSERT_EQ(DState::ERROR, blob9.decodeRESP3(&buf));
    ASSERT_EQ("", blob9.getValue());

    BlobErrorPacket blob10;
    buf.clear();
    buf.append("!6\r\ntest\r\n\r\n!8\r\n\r\ntest\r\n\r\n");
    ASSERT_EQ(DState::SUCCESS, blob10.decodeRESP3(&buf));
    ASSERT_EQ("test\r\n", blob10.getValue());
    ASSERT_EQ(12, blob10.getRESP3EncodeSize());
    ASSERT_EQ(DState::SUCCESS, blob10.decodeRESP3(&buf));
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ("\r\ntest\r\n", blob10.getValue());
    ASSERT_EQ(14, blob10.getRESP3EncodeSize());
}
