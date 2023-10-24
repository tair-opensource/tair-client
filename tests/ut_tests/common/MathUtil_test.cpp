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

#include "common/MathUtil.hpp"

using tair::common::MathUtil;

TEST(MATH_UTIL_IS_POW_OF_TWO_TEST, ONLY_TEST) {
    for (size_t i = 0; i < 64; ++i) {
        ASSERT_TRUE(MathUtil::isPowOfTwo(1u << i));
    }
    ASSERT_FALSE(MathUtil::isPowOfTwo(3));
    ASSERT_FALSE(MathUtil::isPowOfTwo(5));
    ASSERT_FALSE(MathUtil::isPowOfTwo(6));
    ASSERT_FALSE(MathUtil::isPowOfTwo(7));
    ASSERT_FALSE(MathUtil::isPowOfTwo(9));
    ASSERT_FALSE(MathUtil::isPowOfTwo(10));
}

TEST(MATH_UTIL_NEXT_POW_OF_TWO_TEST, ONLY_TEST) {
    ASSERT_EQ(1, MathUtil::nextPowOfTwo(0));
    ASSERT_EQ(1, MathUtil::nextPowOfTwo(1));
    ASSERT_EQ(2, MathUtil::nextPowOfTwo(2));
    ASSERT_EQ(4, MathUtil::nextPowOfTwo(3));
    ASSERT_EQ(4, MathUtil::nextPowOfTwo(4));
    ASSERT_EQ(8, MathUtil::nextPowOfTwo(8));
    ASSERT_EQ(8, MathUtil::nextPowOfTwo(8));
}

TEST(BUILTIN_CLZLL_TEST, ONLY_TEST) {
    ASSERT_EQ(64, __builtin_clzll(0));
    ASSERT_EQ(63, __builtin_clzll(1));
    ASSERT_EQ(62, __builtin_clzll(2));
    ASSERT_EQ(62, __builtin_clzll(3));
    ASSERT_EQ(61, __builtin_clzll(4));
    ASSERT_EQ(61, __builtin_clzll(5));
    ASSERT_EQ(61, __builtin_clzll(6));
    ASSERT_EQ(61, __builtin_clzll(7));
    ASSERT_EQ(60, __builtin_clzll(8));

    for (size_t i = 1; i < 64; ++i) {
        ASSERT_EQ(64 - i - 1, __builtin_clzll(1LU << i));
    }
}
