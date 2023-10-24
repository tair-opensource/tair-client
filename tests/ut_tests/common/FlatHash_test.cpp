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

#include "absl/container/flat_hash_map.h"

TEST(FLAT_HASH_TEST, ONLY_TEST) {
    absl::flat_hash_map<std::string, std::string> map;

    auto pair = map.emplace("k", "v1");
    ASSERT_EQ(true, pair.second);
    ASSERT_EQ(map["k"], "v1");

    pair = map.emplace("k", "v2");
    ASSERT_EQ(false, pair.second);
    ASSERT_EQ(map["k"], "v1");

    pair = map.insert(std::make_pair("k", "v2"));
    ASSERT_EQ(false, pair.second);
    ASSERT_EQ(map["k"], "v1");

    map["k"] = "v2";
    ASSERT_EQ(map["k"], "v2");
}
