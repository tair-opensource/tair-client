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

#include <climits>

#include "network/Buffer.hpp"

using tair::network::Buffer;

TEST(BUFFER_TEST, ONLY_TEST) {
    Buffer buffer(Buffer::GENERAL_BUFFER, 2, 0);

    ASSERT_EQ(0U, buffer.length());
    ASSERT_EQ(2U, buffer.capacity());

    std::string str = "abc";
    buffer.append(str);

    ASSERT_LT(2U, buffer.capacity());
    ASSERT_EQ(str.size(), buffer.size());
    ASSERT_EQ(str, buffer.nextAllString());
    ASSERT_EQ(0U, buffer.length());

    int8_t i8 = 1;
    int16_t i16 = 2;
    int32_t i32 = 3;
    int64_t i64 = 4;

    buffer.appendInt8(i8);
    buffer.appendInt16(i16);
    buffer.appendInt32(i32);
    buffer.appendInt64(i64);

    ASSERT_EQ(sizeof(i8) + sizeof(i16) + sizeof(i32) + sizeof(i64), buffer.size());
    ASSERT_EQ(i8, buffer.readInt8());
    ASSERT_EQ(i16, buffer.readInt16());
    ASSERT_EQ(i32, buffer.readInt32());
    ASSERT_EQ(i64, buffer.readInt64());

    ASSERT_EQ(0U, buffer.size());
}

TEST(BUFFER_TEST, TEST_BUFFER_APPENDREAD) {
    size_t prepend_size = 16;
    size_t init_size = 1024 - prepend_size;
    Buffer buf(Buffer::GENERAL_BUFFER, init_size, prepend_size);

    ASSERT_EQ(0U, buf.length());
    ASSERT_EQ(init_size, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    const std::string str(200, 'x');
    buf.append(str);
    ASSERT_EQ(str.size(), buf.length());
    ASSERT_EQ(init_size - str.size(), buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    const std::string str2 = buf.nextString(50);
    ASSERT_EQ(50U, str2.size());
    ASSERT_EQ(str.size() - str2.size(), buf.length());
    ASSERT_EQ(init_size - str.size(), buf.writableBytes());
    ASSERT_EQ(prepend_size + str2.size(), buf.prependableBytes());
    ASSERT_EQ(std::string(50, 'x'), str2);

    buf.append(str);
    ASSERT_EQ(2 * str.size() - str2.size(), buf.length());
    ASSERT_EQ(init_size - 2 * str.size(), buf.writableBytes());
    ASSERT_EQ(prepend_size + str2.size(), buf.prependableBytes());

    const std::string str3 = buf.nextAllString();
    ASSERT_EQ(350U, str3.size());
    ASSERT_EQ(0U, buf.length());
    ASSERT_EQ(init_size, buf.writableBytes());
    ASSERT_EQ(buf.prependableBytes(), prepend_size);
    ASSERT_EQ(str3, std::string(350, 'x'));
}

TEST(BUFFER_TEST, TEST_BUFFER_GROW1) {
    size_t prepend_size = 8;
    size_t init_size = 1024 - prepend_size;
    Buffer buf(Buffer::GENERAL_BUFFER, init_size, prepend_size);

    buf.append(std::string(400, 'y'));
    ASSERT_EQ(400U, buf.length());
    ASSERT_EQ(init_size - 400, buf.writableBytes());

    buf.skip(50);
    ASSERT_EQ(350U, buf.length());
    ASSERT_EQ(init_size - 400, buf.writableBytes());
    ASSERT_EQ(prepend_size + 50, buf.prependableBytes());

    buf.append(std::string(1000, 'z'));
    ASSERT_EQ(1350U, buf.length());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.reset();
    ASSERT_EQ(0U, buf.length());
    ASSERT_EQ(prepend_size, buf.prependableBytes());
    ASSERT_TRUE(buf.writableBytes() >= init_size * 2);
}

TEST(BUFFER_TEST, TEST_BUFFER_GROW2) {
    size_t prepend_size = 16;
    size_t init_size = 1024 - prepend_size;
    Buffer buf(Buffer::GENERAL_BUFFER, init_size, prepend_size);

    buf.append(std::string(400, 'y'));
    ASSERT_EQ(400U, buf.length());
    ASSERT_EQ(init_size - 400, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.skip(50);
    ASSERT_EQ(350U, buf.length());
    ASSERT_EQ(init_size - 400, buf.writableBytes());
    ASSERT_EQ(prepend_size + 50, buf.prependableBytes());

    buf.append(std::string(1000, 'z'));
    ASSERT_EQ(1350U, buf.length());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.reset();
    ASSERT_EQ(0U, buf.length());
    ASSERT_EQ(prepend_size, buf.prependableBytes());
    ASSERT_TRUE(buf.writableBytes() >= init_size * 2);
}

TEST(BUFFER_TEST, TEST_BUFFER_INSIDEGROW) {
    size_t prepend_size = 16;
    size_t init_size = 1024 - prepend_size;
    Buffer buf(Buffer::GENERAL_BUFFER, init_size, prepend_size);

    buf.append(std::string(800, 'y'));
    ASSERT_EQ(800U, buf.length());
    ASSERT_EQ(init_size - 800, buf.writableBytes());

    buf.skip(500);
    ASSERT_EQ(300U, buf.length());
    ASSERT_EQ(init_size - 800, buf.writableBytes());
    ASSERT_EQ(prepend_size + 500, buf.prependableBytes());

    buf.append(std::string(300, 'z'));
    ASSERT_EQ(600U, buf.length());
    ASSERT_EQ(init_size - 600, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());
}

TEST(BUFFER_TEST, TEST_BUFFER_SHRINK) {
    size_t prepend_size = 16;
    size_t init_size = 1024 - prepend_size;
    Buffer buf(Buffer::GENERAL_BUFFER, init_size, prepend_size);

    buf.append(std::string(2000, 'y'));
    ASSERT_EQ(2000U, buf.length());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.skip(1500);
    ASSERT_EQ(500U, buf.length());
    ASSERT_EQ(prepend_size + 1500, buf.prependableBytes());

    buf.shrink(0);
    ASSERT_EQ(500U, buf.length());
    ASSERT_EQ(0U, buf.writableBytes());
    ASSERT_EQ(std::string(500, 'y'), buf.nextAllString());
    ASSERT_EQ(prepend_size, buf.prependableBytes());
}

TEST(BUFFER_TEST, TEST_BUFFER_PREPEND) {
    size_t prepend_size = 16;
    size_t init_size = 1024 - prepend_size;
    Buffer buf(Buffer::GENERAL_BUFFER, init_size, prepend_size);

    buf.append(std::string(200, 'y'));
    ASSERT_EQ(200U, buf.length());
    ASSERT_EQ(init_size - 200, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    int x = 0;
    buf.prepend(&x, sizeof x);
    ASSERT_EQ(204U, buf.length());
    ASSERT_EQ(init_size - 200, buf.writableBytes());
    ASSERT_EQ(prepend_size - 4, buf.prependableBytes());
}

TEST(BUFFER_TEST, TEST_BUFFER_READINT) {
    size_t prepend_size = 16;
    size_t init_size = 1024 - prepend_size;
    Buffer buf(Buffer::GENERAL_BUFFER, init_size, prepend_size);

    buf.append("HTTP");

    ASSERT_EQ(4U, buf.length());
    ASSERT_EQ('H', buf.peekInt8());
    int top16 = buf.peekInt16();
    ASSERT_EQ(top16, 'H' * 256 + 'T');
    ASSERT_EQ(top16 * 65536 + 'T' * 256 + 'P', buf.peekInt32());

    ASSERT_EQ('H', buf.readInt8());
    ASSERT_EQ('T' * 256 + 'T', buf.readInt16());
    ASSERT_EQ('P', buf.readInt8());
    ASSERT_EQ(0U, buf.length());
    ASSERT_EQ(init_size, buf.writableBytes());

    buf.appendInt8(-1);
    buf.appendInt16(-2);
    buf.appendInt32(-3);
    ASSERT_EQ(7U, buf.length());
    ASSERT_EQ(-1, buf.readInt8());
    ASSERT_EQ(-2, buf.readInt16());
    ASSERT_EQ(-3, buf.readInt32());
    ASSERT_EQ(0U, buf.length());
}

TEST(BUFFER_TEST, TEST_BUFFERF_INDEOL) {
    Buffer buf;
    buf.append(std::string(100000, 'x'));
    ASSERT_EQ(nullptr, buf.findEOL());
    ASSERT_EQ(nullptr, buf.findEOL(buf.data() + 90000));
}

TEST(BUFFER_TEST, TEST_BUFFER_TRUNCATE) {
    size_t prepend_size = 16;
    size_t init_size = 1024 - prepend_size;
    Buffer buf(Buffer::GENERAL_BUFFER, init_size, prepend_size);

    buf.append("HTTP");

    ASSERT_EQ(4U, buf.length());
    buf.truncate(3);
    ASSERT_EQ(3U, buf.length());
    buf.truncate(2);
    ASSERT_EQ(2U, buf.length());
    buf.truncate(1);
    ASSERT_EQ(1U, buf.length());
    ASSERT_EQ(init_size - 1, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());
    buf.truncate(0);
    ASSERT_EQ(0U, buf.length());
    ASSERT_EQ(init_size, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.append("HTTP");
    buf.reset();
    ASSERT_EQ(0U, buf.length());
    ASSERT_EQ(init_size, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.append("HTTP");
    buf.truncate(init_size + 1000);
    ASSERT_EQ(4U, buf.length());
    ASSERT_EQ(init_size - 4, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.append("HTTPS");
    ASSERT_EQ(9U, buf.length());
    ASSERT_EQ(init_size - 9, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.next(4);
    ASSERT_EQ(5U, buf.length());
    ASSERT_EQ(init_size - 9, buf.writableBytes());
    ASSERT_EQ(prepend_size + 4, buf.prependableBytes());
    buf.truncate(5);
    ASSERT_EQ(5U, buf.length());
    ASSERT_EQ(init_size - 9, buf.writableBytes());
    ASSERT_EQ(prepend_size + 4, buf.prependableBytes());
    buf.truncate(6);
    ASSERT_EQ(5U, buf.length());
    ASSERT_EQ(init_size - 9, buf.writableBytes());
    ASSERT_EQ(prepend_size + 4, buf.prependableBytes());
    buf.truncate(4);
    ASSERT_EQ(4U, buf.length());
    ASSERT_EQ(init_size - 8, buf.writableBytes());
    ASSERT_EQ(prepend_size + 4, buf.prependableBytes());
}

TEST(BUFFER_TEST, TEST_BUFFE_RRESERVE) {
    size_t prepend_size = 16;
    size_t init_size = 1024 - prepend_size;
    Buffer buf(Buffer::GENERAL_BUFFER, init_size, prepend_size);

    buf.append("HTTP");
    ASSERT_EQ(4U, buf.length());
    ASSERT_EQ(init_size - 4, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.reserve(100);
    ASSERT_EQ(4U, buf.length());
    ASSERT_EQ(init_size - 4, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.reserve(init_size);
    ASSERT_EQ(4U, buf.length());
    ASSERT_EQ(init_size - 4, buf.writableBytes());
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.reserve(2 * init_size);
    ASSERT_EQ(4U, buf.length());
    ASSERT_TRUE(buf.writableBytes() >= 2 * init_size - 4);
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.append("HTTPS");
    ASSERT_EQ(9U, buf.length());
    ASSERT_TRUE(buf.writableBytes() >= 2 * init_size - 9);
    ASSERT_EQ(prepend_size, buf.prependableBytes());

    buf.next(4);
    ASSERT_EQ(5U, buf.length());
    ASSERT_EQ(prepend_size + 4, buf.prependableBytes());

    buf.reserve(8 * init_size);
    ASSERT_EQ(5U, buf.length());
    ASSERT_TRUE(buf.writableBytes() >= 8 * init_size - 5);
    ASSERT_EQ(prepend_size, buf.prependableBytes());
}

TEST(BUFFER_TEST, COPY_AND_MOVE_TEST) {
    Buffer buf1(Buffer::INPUT_BUFFER);

    buf1.append("hello");
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf1.getType());

    Buffer buf2(buf1);
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf2.getType());
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf1.getType());

    Buffer buf3 = buf2;
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf3.getType());

    Buffer buf4 {buf3};
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf4.getType());

    Buffer buf5(Buffer::GENERAL_BUFFER);
    ASSERT_EQ(Buffer::GENERAL_BUFFER, buf5.getType());

    buf5 = buf4;
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf5.getType());
    ASSERT_EQ(5, buf1.size());
    ASSERT_EQ(5, buf2.size());
    ASSERT_EQ(5, buf3.size());
    ASSERT_EQ(5, buf4.size());
    ASSERT_EQ(5, buf5.size());

    Buffer buf6;
    ASSERT_EQ(Buffer::GENERAL_BUFFER, buf6.getType());
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf1.getType());

    buf6.swap(buf1);
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf6.getType());
    ASSERT_EQ(Buffer::GENERAL_BUFFER, buf1.getType());

    ASSERT_EQ(5, buf6.size());
    ASSERT_EQ(0, buf1.size());

    ASSERT_EQ(Buffer::INPUT_BUFFER, buf2.getType());

    Buffer buf7 = std::move(buf2);
    ASSERT_EQ(Buffer::GENERAL_BUFFER, buf2.getType());
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf7.getType());

    ASSERT_EQ(5, buf7.size());
    ASSERT_EQ(0, buf2.size());

    ASSERT_EQ(Buffer::INPUT_BUFFER, buf3.getType());

    Buffer buf8 {std::move(buf3)};
    ASSERT_EQ(Buffer::GENERAL_BUFFER, buf3.getType());
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf8.getType());

    ASSERT_EQ(5, buf8.size());
    ASSERT_EQ(0, buf3.size());

    Buffer buf9;
    ASSERT_EQ(Buffer::GENERAL_BUFFER, buf9.getType());
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf4.getType());
    buf9 = std::move(buf4);
    ASSERT_EQ(Buffer::INPUT_BUFFER, buf9.getType());
    ASSERT_EQ(Buffer::GENERAL_BUFFER, buf4.getType());

    ASSERT_EQ(5, buf9.size());
    ASSERT_EQ(0, buf4.size());
}

TEST(BUFFER_TEST, APPEND_NUMBER_TO_STR_TEST) {
    Buffer buf;

    buf.appendNumberToStr(-1);
    ASSERT_EQ(2, buf.size());
    ASSERT_EQ("-1", buf.toString());

    buf.clear();
    buf.appendNumberToStr(INT_MAX);
    ASSERT_EQ(10, buf.size());
    ASSERT_EQ("2147483647", buf.toString());

    buf.clear();
    buf.appendNumberToStr(INT_MIN);
    ASSERT_EQ(11, buf.size());
    ASSERT_EQ("-2147483648", buf.toString());

    buf.clear();
    buf.appendNumberToStr(0);
    ASSERT_EQ(1, buf.size());
    ASSERT_EQ("0", buf.toString());

    buf.clear();
    buf.appendNumberToStr(INT64_MAX);
    ASSERT_EQ(19, buf.size());
    ASSERT_EQ("9223372036854775807", buf.toString());

    buf.clear();
    buf.appendNumberToStr(INT64_MIN);
    ASSERT_EQ(20, buf.size());
    ASSERT_EQ("-9223372036854775808", buf.toString());

    buf.clear();
    buf.appendNumberToStr(0);
    ASSERT_EQ(1, buf.size());
    ASSERT_EQ("0", buf.toString());

    buf.clear();
    buf.appendNumberToStr(UINT64_MAX);
    ASSERT_EQ(20, buf.size());
    ASSERT_EQ("18446744073709551615", buf.toString());
}

TEST(BUFFER_TEST, TO_DEBUG_STRING_TEST) {
    Buffer buf;

    ASSERT_EQ("ridx: 0, widx: 0, data: ", buf.toDebugString(1024));

    buf.append("test_data");
    ASSERT_EQ("ridx: 0, widx: 9, data: test", buf.toDebugString(4));
    ASSERT_EQ("ridx: 0, widx: 9, data: test_dat", buf.toDebugString(8));
    ASSERT_EQ("ridx: 0, widx: 9, data: test_data", buf.toDebugString(9));
    ASSERT_EQ("ridx: 0, widx: 9, data: test_data", buf.toDebugString(10));
    ASSERT_EQ("ridx: 0, widx: 9, data: test_data", buf.toDebugString(1024));

    buf.skip(2);

    ASSERT_EQ("ridx: 2, widx: 9, data: test", buf.toDebugString(4));
    ASSERT_EQ("ridx: 2, widx: 9, data: test_dat", buf.toDebugString(8));
    ASSERT_EQ("ridx: 2, widx: 9, data: test_data", buf.toDebugString(9));
    ASSERT_EQ("ridx: 2, widx: 9, data: test_data", buf.toDebugString(10));
    ASSERT_EQ("ridx: 2, widx: 9, data: test_data", buf.toDebugString(1024));
}