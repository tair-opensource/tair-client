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

#include "client/params/ParamsAll.hpp"
#include "TairClient_Standalone_Server.hpp"

using tair::client::ZAddParams;
using tair::client::ZInterUnionParams;
using tair::client::ZRangeParams;
using tair::client::ZRemRangeOption;

TEST_F(StandAloneTest, ZADDCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(4, wrapper.zadd("myzset", {"1", "one", "1", "uno", "2", "two", "3", "three"}).get().getValue());
    auto ret = wrapper.zrange("myzset", "0", "-1").get().getValue();
    ASSERT_EQ(4, ret.size());
    ASSERT_EQ("one", ret[0]);
    ASSERT_EQ("uno", ret[1]);
    ASSERT_EQ("two", ret[2]);
    ASSERT_EQ("three", ret[3]);
}

TEST_F(StandAloneTest, ZINCRBYCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.zadd("myzset", {"1", "one"}).get().getValue());
    ASSERT_EQ("3", wrapper.zincrby("myzset", 2, "one").get().getValue());
}

TEST_F(StandAloneTest, ZSCORECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.zadd("myzset", {"1", "one"}).get().getValue());
    ASSERT_EQ("1", *wrapper.zscore("myzset", "one").get().getValue());
    ASSERT_EQ(nullptr, wrapper.zscore("myzset", "not-exists").get().getValue());
}

TEST_F(StandAloneTest, ZRANKCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("myzset", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ASSERT_EQ(2, *wrapper.zrank("myzset", "three").get().getValue());
    ASSERT_EQ(nullptr, wrapper.zrank("myzset", "four").get().getValue());
}

TEST_F(StandAloneTest, ZREVRANKCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("myzset", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ASSERT_EQ(2, *wrapper.zrevrank("myzset", "one").get().getValue());
    ASSERT_EQ(nullptr, wrapper.zrank("myzset", "four").get().getValue());
}

TEST_F(StandAloneTest, ZCARDCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("myzset", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ASSERT_EQ(3, wrapper.zcard("myzset").get().getValue());
}

TEST_F(StandAloneTest, ZCOUNTCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("myzset", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ASSERT_EQ(3, wrapper.zcount("myzset", "-inf", "+inf").get().getValue());
    ASSERT_EQ(2, wrapper.zcount("myzset", "(1", "3").get().getValue());
}

TEST_F(StandAloneTest, ZLEXCOUNTCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(5, wrapper.zadd("myzset", {"0", "a", "0", "b", "0", "c", "0", "d", "0", "e"}).get().getValue());
    ASSERT_EQ(2, wrapper.zadd("myzset", {"0", "f", "0", "g"}).get().getValue());
    ASSERT_EQ(7, wrapper.zlexcount("myzset", "-", "+").get().getValue());
    ASSERT_EQ(5, wrapper.zlexcount("myzset", "[b", "[f").get().getValue());
}

TEST_F(StandAloneTest, ZPOPMAXCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("myzset", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    auto ret = wrapper.zpopmax("myzset", 1).get().getValue();
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("three", ret[0]);
    ASSERT_EQ("3", ret[1]);
    ret.clear();
    ret = wrapper.zpopmax("myzset", 3).get().getValue();
    ASSERT_EQ(4, ret.size());
    ret.clear();
    ret = wrapper.zpopmax("myzset", 3).get().getValue();
    ASSERT_EQ(0, ret.size());
}

TEST_F(StandAloneTest, BZPOPMAXCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    auto ret = wrapper.bzpopmax({"zset1", "zset2"}, 1).get().getValue();
    ASSERT_EQ(0, ret.size());
    ASSERT_EQ(3, wrapper.zadd("zset1", {"0", "a", "1", "b", "2", "c"}).get().getValue());
    ret = wrapper.bzpopmax({"zset1", "zset2"}, 1).get().getValue();
    ASSERT_EQ(3, ret.size());
}

TEST_F(StandAloneTest, ZRANGECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("myzset", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    auto ret = wrapper.zrange("myzset", "0", "-1").get().getValue();
    ASSERT_EQ(3, ret.size());
    ret.clear();
    ret = wrapper.zrange("myzset", "2", "3").get().getValue();
    ASSERT_EQ(1, ret.size());
    ret = wrapper.zrange("myzset", "-2", "-1").get().getValue();
    ASSERT_EQ(2, ret.size());
    // with params
    ZRangeParams params;
    ret.clear();
    ret = wrapper.zrange("myzset", "0", "1", params.withscores()).get().getValue();
    ASSERT_EQ(4, ret.size());
}

TEST_F(StandAloneTest, ZRANGESTORECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(4, wrapper.zadd("myzset", {"1", "one", "2", "two", "3", "three", "4", "four"}).get().getValue());
    ASSERT_EQ(2, wrapper.zrangestore("dstzset", "myzset", "2", "-1").get().getValue());
    ASSERT_EQ(2, wrapper.zcard("dstzset").get().getValue());
}

TEST_F(StandAloneTest, ZREMCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("myzset", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ASSERT_EQ(1, wrapper.zrem("myzset", {"two"}).get().getValue());
}

TEST_F(StandAloneTest, ZREMRANGEBYLEXCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(5, wrapper.zadd("myzset", {"0", "aaaa", "0", "b", "0", "c", "0", "d", "0", "e"}).get().getValue());
    ASSERT_EQ(5, wrapper.zadd("myzset", {"0", "foo", "0", "zap", "0", "zip", "0", "ALPHA", "0", "alpha"}).get().getValue());
    ASSERT_EQ(6, wrapper.zremrange(ZRemRangeOption::BYLEX, "myzset", "[alpha", "[omega").get().getValue());
    auto ret = wrapper.zrange("myzset", "0", "-1").get().getValue();
    ASSERT_EQ(4, ret.size());
}

TEST_F(StandAloneTest, ZREMRANGEBYRANKCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("myzset", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ASSERT_EQ(2, wrapper.zremrange(ZRemRangeOption::BYRANK, "myzset", "0", "1").get().getValue());
}

TEST_F(StandAloneTest, ZREMRANGEBYSCORECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("myzset", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ASSERT_EQ(1, wrapper.zremrange(ZRemRangeOption::BYSCORE, "myzset", "-inf", "(2").get().getValue());
}

TEST_F(StandAloneTest, ZINTERCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.zadd("zset1", {"1", "one", "2", "two"}).get().getValue());
    ASSERT_EQ(3, wrapper.zadd("zset2", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    auto ret = wrapper.zinter({"zset1", "zset2"}).get().getValue();
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("one", ret[0]);
    ASSERT_EQ("two", ret[1]);
}

TEST_F(StandAloneTest, ZINTERSTORECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.zadd("zset1", {"1", "one", "2", "two"}).get().getValue());
    ASSERT_EQ(3, wrapper.zadd("zset2", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ZInterUnionParams params;
    ASSERT_EQ(2, wrapper.zinterstore("out", {"zset1", "zset2"}, params.weights({2, 3})).get().getValue());
    ZRangeParams rangeParams;
    auto ret = wrapper.zrange("out", "0", "-1", rangeParams.withscores()).get().getValue();
    ASSERT_EQ(4, ret.size());
    ASSERT_EQ("one", ret[0]);
    ASSERT_EQ("5", ret[1]);
    ASSERT_EQ("two", ret[2]);
    ASSERT_EQ("10", ret[3]);
}

TEST_F(StandAloneTest, ZUNIONCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.zadd("zset1", {"1", "one", "2", "two"}).get().getValue());
    ASSERT_EQ(3, wrapper.zadd("zset2", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    auto ret = wrapper.zunion({"zset1", "zset2"}).get().getValue();
    ASSERT_EQ(3, ret.size());
}

TEST_F(StandAloneTest, ZUNIONSTORECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.zadd("zset1", {"1", "one", "2", "two"}).get().getValue());
    ASSERT_EQ(3, wrapper.zadd("zset2", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ASSERT_EQ(3, wrapper.zunionstore("out", {"zset1", "zset2"}).get().getValue());
}

TEST_F(StandAloneTest, ZDIFFCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("zset1", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ASSERT_EQ(2, wrapper.zadd("zset2", {"1", "one", "2", "two"}).get().getValue());
    auto ret = wrapper.zdiff({"zset1", "zset2"}).get().getValue();
    ASSERT_EQ(1, ret.size());
}

TEST_F(StandAloneTest, ZDIFFSTORECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.zadd("zset1", {"1", "one", "2", "two", "3", "three"}).get().getValue());
    ASSERT_EQ(2, wrapper.zadd("zset2", {"1", "one", "2", "two"}).get().getValue());
    ASSERT_EQ(1, wrapper.zdiffstore("out", {"zset1", "zset2"}).get().getValue());
}

TEST_F(StandAloneTest, ZMSCORECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.zadd("myzset", {"1", "one", "2", "two"}).get().getValue());
    auto ret = wrapper.zmscore("myzset", {"one", "two", "not-exists"}).get().getValue();
    ASSERT_EQ(3, ret.size());
    ASSERT_EQ("1", *ret[0]);
    ASSERT_EQ("2", *ret[1]);
    ASSERT_EQ(nullptr, ret[2]);
}

TEST_F(StandAloneTest, ZRANDMEMBERCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.zadd("myzset", {"1", "one", "2", "two"}).get().getValue());
    ASSERT_TRUE(wrapper.zrandmember("myzset").get().getValue() != nullptr);
    auto ret = wrapper.zrandmember("myzset", 2).get().getValue();
    ASSERT_EQ(2, ret.size());
    ret.clear();
    ret = wrapper.zrandmember("myzset", -5).get().getValue();
    ASSERT_EQ(5, ret.size());
}

TEST_F(StandAloneTest, ZSCANCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ(1, wrapper.zadd("myzset", {std::to_string(i), std::to_string(i)}).get().getValue());
    }

    std::vector<std::string> all_fields;
    std::string cursor = "0";
    while (true) {
        auto result = wrapper.zscan("myzset", cursor).get().getValue();
        cursor = result.cursor;
        all_fields.insert(all_fields.end(), result.results.begin(), result.results.end());
        if (cursor == "0") {
            break;
        }
    }

    std::sort(all_fields.begin(), all_fields.end(), [](std::string a, std::string b) {
        return std::stoi(a) < std::stoi(b);
    });
    for (int i = 0; i < 200; i += 2) {
        ASSERT_EQ(std::to_string(i / 2), all_fields[i]);
    }
}
