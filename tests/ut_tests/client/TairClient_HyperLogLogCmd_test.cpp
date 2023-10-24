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

#include <algorithm>

#include "TairClient_Standalone_Server.hpp"

TEST_F(StandAloneTest, PFADDCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.pfadd("hll", {"a", "b", "c", "d", "e", "f", "g"}).get().getValue());
    ASSERT_EQ(7, wrapper.pfcount({"hll"}).get().getValue());
}

TEST_F(StandAloneTest, PFMERGECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.pfadd("hll1", {"foo", "bar", "zap", "a"}).get().getValue());
    ASSERT_EQ(1, wrapper.pfadd("hll2", {"a", "b", "c", "foo"}).get().getValue());
    ASSERT_EQ("OK", wrapper.pfmerge("hll3", {"hll1", "hll2"}).get().getValue());
    ASSERT_EQ(6, wrapper.pfcount({"hll3"}).get().getValue());
}