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

TEST_F(StandAloneTest, SADDCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("set", {"a", "b", "c", "c"}).get().getValue());
    auto ret = wrapper.smembers("set").get().getValue();
    std::sort(ret.begin(), ret.end(), [](std::string a, std::string b) {
        return a < b;
    });
    ASSERT_EQ(3, ret.size());
    ASSERT_EQ("a", ret[0]);
    ASSERT_EQ("b", ret[1]);
    ASSERT_EQ("c", ret[2]);
}

TEST_F(StandAloneTest, SCARDCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("set", {"a", "b", "c"}).get().getValue());
    ASSERT_EQ(3, wrapper.scard("set").get().getValue());
}

TEST_F(StandAloneTest, SISMEMBERCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("set", {"a", "b", "c"}).get().getValue());
    ASSERT_EQ(1, wrapper.sismember("set", "a").get().getValue());
    ASSERT_EQ(0, wrapper.sismember("set", "d").get().getValue());
}

TEST_F(StandAloneTest, SMOVECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.sadd("myset", {"one", "two"}).get().getValue());
    ASSERT_EQ(1, wrapper.sadd("myotherset", {"three"}).get().getValue());
    ASSERT_EQ(1, wrapper.smove("myset", "myotherset", "two").get().getValue());
    ASSERT_EQ(1, wrapper.scard("myset").get().getValue());
    ASSERT_EQ(2, wrapper.scard("myotherset").get().getValue());
}

TEST_F(StandAloneTest, SREMCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("myset", {"one", "two", "three"}).get().getValue());
    ASSERT_EQ(1, wrapper.srem("myset", {"one"}).get().getValue());
    ASSERT_EQ(0, wrapper.srem("myset", {"four"}).get().getValue());
    ASSERT_EQ(2, wrapper.scard("myset").get().getValue());
}

TEST_F(StandAloneTest, SPOPCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("myset", {"one", "two", "three"}).get().getValue());
    ASSERT_TRUE(wrapper.spop("myset").get().getValue() != nullptr);
    ASSERT_EQ(2, wrapper.scard("myset").get().getValue());
    ASSERT_EQ(2, wrapper.sadd("myset", {"four", "five"}).get().getValue());
    auto ret = wrapper.spop("myset", 3).get().getValue();
    ASSERT_EQ(3, ret.size());
    ASSERT_EQ(1, wrapper.scard("myset").get().getValue());
}

TEST_F(StandAloneTest, SRANDMEMBERCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("myset", {"one", "two", "three"}).get().getValue());
    ASSERT_TRUE(wrapper.srandmember("myset").get().getValue() != nullptr);
    auto ret = wrapper.srandmember("myset", 2).get().getValue();
    ASSERT_EQ(2, ret.size());
    ret.clear();
    ret = wrapper.srandmember("myset", -5).get().getValue();
    ASSERT_EQ(5, ret.size());
}

TEST_F(StandAloneTest, SDIFFCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("key1", {"a", "b", "c"}).get().getValue());
    ASSERT_EQ(3, wrapper.sadd("key2", {"c", "d", "e"}).get().getValue());
    auto ret = wrapper.sdiff({"key1", "key2"}).get().getValue();
    std::sort(ret.begin(), ret.end(), [](std::string a, std::string b) {
        return a < b;
    });
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("a", ret[0]);
    ASSERT_EQ("b", ret[1]);
}

TEST_F(StandAloneTest, SINTERCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("key1", {"a", "b", "c"}).get().getValue());
    ASSERT_EQ(3, wrapper.sadd("key2", {"c", "d", "e"}).get().getValue());
    auto ret = wrapper.sinter({"key1", "key2"}).get().getValue();
    std::sort(ret.begin(), ret.end(), [](std::string a, std::string b) {
        return a < b;
    });
    ASSERT_EQ(1, ret.size());
    ASSERT_EQ("c", ret[0]);
}

TEST_F(StandAloneTest, SUNIONCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("key1", {"a", "b", "c"}).get().getValue());
    ASSERT_EQ(3, wrapper.sadd("key2", {"c", "d", "e"}).get().getValue());
    auto ret = wrapper.sunion({"key1", "key2"}).get().getValue();
    std::sort(ret.begin(), ret.end(), [](std::string a, std::string b) {
        return a < b;
    });
    ASSERT_EQ(5, ret.size());
    ASSERT_EQ("a", ret[0]);
    ASSERT_EQ("b", ret[1]);
    ASSERT_EQ("c", ret[2]);
    ASSERT_EQ("d", ret[3]);
    ASSERT_EQ("e", ret[4]);
}

TEST_F(StandAloneTest, SDIFFSTORECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("key1", {"a", "b", "c"}).get().getValue());
    ASSERT_EQ(3, wrapper.sadd("key2", {"c", "d", "e"}).get().getValue());
    ASSERT_EQ(2, wrapper.sdiffstore("key", {"key1", "key2"}).get().getValue());
    auto ret = wrapper.smembers("key").get().getValue();
    std::sort(ret.begin(), ret.end(), [](std::string a, std::string b) {
        return a < b;
    });
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("a", ret[0]);
    ASSERT_EQ("b", ret[1]);
}

TEST_F(StandAloneTest, SINTERSTORECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("key1", {"a", "b", "c"}).get().getValue());
    ASSERT_EQ(3, wrapper.sadd("key2", {"c", "d", "e"}).get().getValue());
    ASSERT_EQ(1, wrapper.sinterstore("key", {"key1", "key2"}).get().getValue());
    auto ret = wrapper.smembers("key").get().getValue();
    std::sort(ret.begin(), ret.end(), [](std::string a, std::string b) {
        return a < b;
    });
    ASSERT_EQ(1, ret.size());
    ASSERT_EQ("c", ret[0]);
}

TEST_F(StandAloneTest, SUNIONSTORECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("key1", {"a", "b", "c"}).get().getValue());
    ASSERT_EQ(3, wrapper.sadd("key2", {"c", "d", "e"}).get().getValue());
    ASSERT_EQ(5, wrapper.sunionstore("key", {"key1", "key2"}).get().getValue());
    auto ret = wrapper.smembers("key").get().getValue();
    std::sort(ret.begin(), ret.end(), [](std::string a, std::string b) {
        return a < b;
    });
    ASSERT_EQ(5, ret.size());
    ASSERT_EQ("a", ret[0]);
    ASSERT_EQ("b", ret[1]);
    ASSERT_EQ("c", ret[2]);
    ASSERT_EQ("d", ret[3]);
    ASSERT_EQ("e", ret[4]);
}

TEST_F(StandAloneTest, SSCANCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ(1, wrapper.sadd("set", {std::to_string(i)}).get().getValue());
    }

    std::vector<std::string> all_fields;
    std::string cursor = "0";
    while (true) {
        auto result = wrapper.sscan("set", cursor).get().getValue();
        cursor = result.cursor;
        all_fields.insert(all_fields.end(), result.results.begin(), result.results.end());
        if (cursor == "0") {
            break;
        }
    }

    std::sort(all_fields.begin(), all_fields.end(), [](std::string a, std::string b) {
        return std::stoi(a) < std::stoi(b);
    });
    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ(std::to_string(i), all_fields[i]);
    }
}

TEST_F(StandAloneTest, SMISMEMBERCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.sadd("key1", {"a", "b", "c"}).get().getValue());
    auto ret = wrapper.smismember("key1", {"a", "d"}).get().getValue();
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ(1, ret[0]);
    ASSERT_EQ(0, ret[1]);
}
