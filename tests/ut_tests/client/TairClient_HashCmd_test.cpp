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

TEST_F(StandAloneTest, HDELCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.hset("myhash", "field1", "foo").get().getValue());
    ASSERT_EQ(1, wrapper.hdel("myhash", {"field1"}).get().getValue());
    ASSERT_EQ(0, wrapper.hdel("myhash", {"field2"}).get().getValue());
}

TEST_F(StandAloneTest, HEXISTSCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.hset("myhash", "field1", "foo").get().getValue());
    ASSERT_EQ(1, wrapper.hexists("myhash", {"field1"}).get().getValue());
    ASSERT_EQ(0, wrapper.hexists("myhash", {"field2"}).get().getValue());
}

TEST_F(StandAloneTest, HGETCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.hset("myhash", "field1", "foo").get().getValue());
    ASSERT_EQ("foo", *wrapper.hget("myhash", "field1").get().getValue());
    ASSERT_EQ(nullptr, wrapper.hget("myhash", "field2").get().getValue());
}

TEST_F(StandAloneTest, HGETALLCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.hset("myhash", {"a", "b", "c", "d"}).get().getValue());
    auto ret = wrapper.hgetall("myhash").get().getValue();
    std::sort(ret.begin(), ret.end(), [](std::string a, std::string b) {
        return a < b;
    });
    ASSERT_EQ(4, ret.size());
    ASSERT_EQ("a", ret[0]);
    ASSERT_EQ("b", ret[1]);
    ASSERT_EQ("c", ret[2]);
    ASSERT_EQ("d", ret[3]);
}

TEST_F(StandAloneTest, HINCRBYCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.hset("myhash", "field", "5").get().getValue());
    ASSERT_EQ(6, wrapper.hincrby("myhash", "field", 1).get().getValue());
    ASSERT_EQ(5, wrapper.hincrby("myhash", "field", -1).get().getValue());
    ASSERT_EQ(-5, wrapper.hincrby("myhash", "field", -10).get().getValue());
}

TEST_F(StandAloneTest, HINCRBYFLOATCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.hset("myhash", "field", "10.50").get().getValue());
    ASSERT_EQ("10.6", wrapper.hincrbyfloat("myhash", "field", 0.1).get().getValue());
    ASSERT_EQ("5.6", wrapper.hincrbyfloat("myhash", "field", -5).get().getValue());
    ASSERT_EQ(0, wrapper.hset("myhash", "field", "5.0e3").get().getValue());
    ASSERT_EQ("5200", wrapper.hincrbyfloat("myhash", "field", 2.0e2).get().getValue());
}

TEST_F(StandAloneTest, HKEYSCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.hset("myhash", {"a", "b", "c", "d"}).get().getValue());
    auto ret = wrapper.hkeys("myhash").get().getValue();
    std::sort(ret.begin(), ret.end(), [](std::string a, std::string b) {
        return a < b;
    });
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("a", ret[0]);
    ASSERT_EQ("c", ret[1]);
}

TEST_F(StandAloneTest, HVALSCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.hset("myhash", {"a", "b", "c", "d"}).get().getValue());
    auto ret = wrapper.hvals("myhash").get().getValue();
    std::sort(ret.begin(), ret.end(), [](std::string a, std::string b) {
        return a < b;
    });
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("b", ret[0]);
    ASSERT_EQ("d", ret[1]);
}

TEST_F(StandAloneTest, HLENCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.hset("myhash", {"a", "b", "c", "d"}).get().getValue());
    ASSERT_EQ(2, wrapper.hlen("myhash").get().getValue());
}

TEST_F(StandAloneTest, HMGETCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.hset("myhash", {"a", "b", "c", "d"}).get().getValue());
    auto ret = wrapper.hmget("myhash", {"a", "c", "notexists"}).get().getValue();
    ASSERT_EQ(3, ret.size());
    ASSERT_EQ("b", *ret[0]);
    ASSERT_EQ("d", *ret[1]);
    ASSERT_EQ(nullptr, ret[2]);
}

TEST_F(StandAloneTest, HSETNXCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.hsetnx("key", "a", "b").get().getValue());
    ASSERT_EQ("b", *wrapper.hget("key", "a").get().getValue());
    ASSERT_EQ(0, wrapper.hsetnx("key", "a", "b").get().getValue());
}

TEST_F(StandAloneTest, HSTRLENCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.hset("myhash", {"a", "b", "c", "d"}).get().getValue());
    ASSERT_EQ(1, wrapper.hstrlen("myhash", "a").get().getValue());
    ASSERT_EQ(0, wrapper.hstrlen("myhash", "not-exists").get().getValue());
}

TEST_F(StandAloneTest, HSCANCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ(1, wrapper.hset("hash", {std::to_string(i), std::to_string(i)}).get().getValue());
    }

    std::vector<std::string> all_fields;
    std::string cursor = "0";
    while (true) {
        auto result = wrapper.hscan("hash", cursor).get().getValue();
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

TEST_F(StandAloneTest, HRANDFIELDCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.hset("coin", {"heads", "obverse", "tails", "reverse", "edge", "null"}).get().getValue());
    auto ret = wrapper.hrandfield("coin", 2).get().getValue();
    ASSERT_EQ(2, ret.size());
    ret.clear();
    ret = wrapper.hrandfield("coin", 10).get().getValue();
    ASSERT_EQ(3, ret.size());
    ret.clear();
    ret = wrapper.hrandfield("coin", -100).get().getValue();
    ASSERT_EQ(100, ret.size());
}
