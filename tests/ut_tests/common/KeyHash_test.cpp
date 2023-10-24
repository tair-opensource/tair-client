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

#include "common/CRC.hpp"
#include "common/KeyHash.hpp"
#include "common/SipHash.hpp"

using tair::common::CRC;
using tair::common::KeyHash;
using tair::common::siphash_nocase;

TEST(CRC_TEST, ONLY_TEST) {
    std::string key = "123456789";
    ASSERT_EQ(0x31c3, CRC::crc16(key.data(), key.size()));
    ASSERT_EQ(0xe9c6d914c4b8d9ca, CRC::crc64(0, key.data(), key.size()));
}

TEST(KEY_HASH_TEST, ONLY_TEST) {
    std::string key1 = "abcde";
    ASSERT_EQ(16097, KeyHash::keyHashSlot(key1.data(), key1.size()));

    std::string key2 = "abcde{";
    ASSERT_EQ(14689, KeyHash::keyHashSlot(key2.data(), key2.size()));

    std::string key3 = "abcde}";
    ASSERT_EQ(6567, KeyHash::keyHashSlot(key3.data(), key3.size()));

    std::string key4 = "{abcde}";
    ASSERT_EQ(16097, KeyHash::keyHashSlot(key4.data(), key4.size()));

    std::string key5 = "abcdefghi{same}abcdefghi";
    std::string key6 = "123456789{same}123456789";
    ASSERT_EQ(KeyHash::keyHashSlot(key5.data(), key5.size()), KeyHash::keyHashSlot(key6.data(), key6.size()));
}

TEST(SIPHASH_TEST, ONLY_TEST) {
    ASSERT_EQ(siphash_nocase((const uint8_t *)"SET", 3), siphash_nocase((const uint8_t *)"set", 3));
}
