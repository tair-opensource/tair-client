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
#include <string>

#include "client/params/ParamsAll.hpp"
#include "TairClient_Standalone_Server.hpp"

using tair::client::ExpireParams;

TEST_F(StandAloneTest, BASIC_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    std::string key = "key", value = "value";
    ASSERT_EQ("OK", wrapper.set(key, value).get().getValue());
    ASSERT_EQ(value, *(wrapper.get(key).get().getValue()));
    ASSERT_EQ(1, wrapper.exists(key).get().getValue());
    ASSERT_EQ(1, wrapper.expire(key, 5).get().getValue());
    ASSERT_GT(wrapper.ttl(key).get().getValue(), 0);
    ASSERT_EQ(1, wrapper.persist(key).get().getValue());
    ASSERT_EQ(-1, wrapper.ttl(key).get().getValue());
    ASSERT_EQ(1, wrapper.del(key).get().getValue());
}

TEST_F(StandAloneTest, EXPIRE_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    std::string key = "key", value = "Hello";
    ASSERT_EQ("OK", wrapper.set(key, value).get().getValue());
    ASSERT_EQ(1, wrapper.expire(key, 10).get().getValue());
    ASSERT_EQ(10, wrapper.ttl(key).get().getValue());
    ASSERT_EQ("OK", wrapper.set(key, "Hello World").get().getValue());
    ASSERT_EQ(-1, wrapper.ttl(key).get().getValue());
    ExpireParams p1;
    ASSERT_EQ(0, wrapper.expire(key, 10, p1.xx()).get().getValue());
    ASSERT_EQ(-1, wrapper.ttl(key).get().getValue());
    ExpireParams p2;
    ASSERT_EQ(1, wrapper.expire(key, 10, p2.nx()).get().getValue());
    ASSERT_EQ(10, wrapper.ttl(key).get().getValue());
}

TEST_F(StandAloneTest, DEL_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.mset({"k1", "v1", "k2", "v2", "k3", "v3"}).get().getValue());
    ASSERT_EQ(1, wrapper.unlink("k1").get().getValue());
    ASSERT_EQ(2, wrapper.unlink({"k2", "k3"}).get().getValue());
    ASSERT_EQ(0, wrapper.exists({"k2", "k3"}).get().getValue());
    ASSERT_EQ(0, wrapper.del({"k1", "k2"}).get().getValue());
}

TEST_F(StandAloneTest, DUMP_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("key", "value").get().getValue());
    auto dump = wrapper.dump("key").get().getValue();
    ASSERT_EQ("OK", wrapper.restore("newkey", 5, *dump).get().getValue());
    ASSERT_GT(5000, wrapper.pttl("newkey").get().getValue());
}

TEST_F(StandAloneTest, EXPIREAT_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("key", "value").get().getValue());
    ASSERT_EQ(1, wrapper.exists("key").get().getValue());
    ASSERT_EQ(1, wrapper.expireat("key", 1293840000).get().getValue());
    ASSERT_EQ(0, wrapper.exists("key").get().getValue());
}

TEST_F(StandAloneTest, PEXPIREAT_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("key", "value").get().getValue());
    ASSERT_EQ(1, wrapper.exists("key").get().getValue());
    ASSERT_EQ(1, wrapper.pexpireat("key", 1555555555005).get().getValue());
    ASSERT_EQ(-2, wrapper.pttl("key").get().getValue());
}

TEST_F(StandAloneTest, TOUCH_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("key1", "value1").get().getValue());
    ASSERT_EQ("OK", wrapper.set("key2", "value2").get().getValue());
    ASSERT_EQ(2, wrapper.touch({"key1", "key2"}).get().getValue());
}

TEST_F(StandAloneTest, RANDOM_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("key", "value").get().getValue());
    ASSERT_EQ("key", *wrapper.randomkey().get().getValue());
}

TEST_F(StandAloneTest, RENAME_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("key", "value").get().getValue());
    ASSERT_EQ("OK", wrapper.rename("key", "newkey").get().getValue());
    ASSERT_EQ("value", *(wrapper.get("newkey").get().getValue()));
    ASSERT_EQ("OK", wrapper.set("oldkey", "value").get().getValue());
    ASSERT_EQ(0, wrapper.renamenx("oldkey", "newkey").get().getValue());
}

TEST_F(StandAloneTest, TYPE_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("key", "value").get().getValue());
    ASSERT_EQ("string", wrapper.type("key").get().getValue());
}

TEST_F(StandAloneTest, KEYS_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();

    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ("OK", wrapper.set(std::to_string(i), std::to_string(i)).get().getValue());
    }

    auto keys = wrapper.keys("*").get().getValue();
    ASSERT_EQ(100, keys.size());

    std::sort(keys.begin(), keys.end(), [](std::string a, std::string b) {
        return std::stoi(a) < std::stoi(b);
    });
    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ(std::to_string(i), keys[i]);
    }
}

TEST_F(StandAloneTest, SCAN_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();

    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ("OK", wrapper.set(std::to_string(i), std::to_string(i)).get().getValue());
    }

    std::vector<std::string> all_keys;
    std::string cursor = "0";
    while (true) {
        auto result = wrapper.scan(cursor).get().getValue();
        cursor = result.cursor;
        all_keys.insert(all_keys.end(), result.results.begin(), result.results.end());
        if (cursor == "0") {
            break;
        }
    }

    std::sort(all_keys.begin(), all_keys.end(), [](std::string a, std::string b) {
        return std::stoi(a) < std::stoi(b);
    });
    for (int i = 0; i < 100; ++i) {
        ASSERT_EQ(std::to_string(i), all_keys[i]);
    }
}

TEST_F(StandAloneTest, COPY_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("key", "value").get().getValue());
    ASSERT_EQ(1, wrapper.copy("key", "newkey").get().getValue());
    ASSERT_EQ("value", *(wrapper.get("newkey").get().getValue()));
}
