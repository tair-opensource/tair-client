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

#include "common/ConcurrentHashMap.hpp"

using tair::common::ConcurrentHashMap;

TEST(CONCURRENT_HASH_MAP_TEST, ONLY_TEST) {
    ConcurrentHashMap<std::string, std::string> map;
    ASSERT_TRUE(map.insert(std::make_pair("k1", "v1")).second);
    ASSERT_FALSE(map.insert(std::make_pair("k1", "v1")).second);
    ASSERT_TRUE(map.insert(std::make_pair("k2", "v2")).second);
    ASSERT_TRUE(map.contains("k1"));
    ASSERT_TRUE(map.contains("k2"));
    ASSERT_FALSE(map.contains("k3"));
    auto [exist1, value1] = map.get("k1");
    ASSERT_TRUE(exist1);
    ASSERT_EQ("v1", value1);
    auto [exist2, value2] = map.get("k2");
    ASSERT_TRUE(exist2);
    ASSERT_EQ("v2", value2);
    auto [exist3, value3] = map.get("k3");
    ASSERT_FALSE(exist3);
    ASSERT_EQ(2U, map.size());
    ASSERT_TRUE(map.erase("k1"));
    ASSERT_EQ(1U, map.size());
    ASSERT_FALSE(map.contains("k1"));
    ASSERT_TRUE(map.contains("k2"));
    map.clear();
    ASSERT_EQ(0U, map.size());
}
