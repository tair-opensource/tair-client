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

using tair::client::ListDirection;

TEST_F(StandAloneTest, BLPOPCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.lpush("list1", {"a", "b", "c"}).get().getValue());
    auto ret = wrapper.blpop({"list1", "list2"}, 0).get().getValue();
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("list1", ret[0]);
    ASSERT_EQ("c", ret[1]);
}

TEST_F(StandAloneTest, BRPOPCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    auto ret = wrapper.brpop({"not-exists-key"}, 1).get().getValue();
    ASSERT_EQ(0, ret.size());
}

TEST_F(StandAloneTest, BRPOPLPUSHCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.rpush("mylist", {"one", "two", "three"}).get().getValue());
    ASSERT_EQ("three", *wrapper.brpoplpush("mylist", "myotherlist", 0).get().getValue());
}

TEST_F(StandAloneTest, LINDEXCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.lpush("mylist", {"World", "Hello"}).get().getValue());
    ASSERT_EQ("Hello", *wrapper.lindex("mylist", 0).get().getValue());
    ASSERT_EQ("World", *wrapper.lindex("mylist", -1).get().getValue());
    ASSERT_EQ(nullptr, wrapper.lindex("mylist", 3).get().getValue());
}

TEST_F(StandAloneTest, LINSERTCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(2, wrapper.rpush("mylist", {"Hello", "World"}).get().getValue());
    ASSERT_EQ(3, wrapper.linsert("mylist", ListDirection::BEFORE, "World", "Three").get().getValue());
    auto ret = wrapper.lrange("mylist", 0, -1).get().getValue();
    ASSERT_EQ(3, ret.size());
    ASSERT_EQ("Hello", ret[0]);
    ASSERT_EQ("Three", ret[1]);
    ASSERT_EQ("World", ret[2]);
}

TEST_F(StandAloneTest, LLENCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(0, wrapper.llen("not-exists-key").get().getValue());
    ASSERT_EQ(2, wrapper.rpush("mylist", {"Hello", "World"}).get().getValue());
    ASSERT_EQ(2, wrapper.llen("mylist").get().getValue());
}

TEST_F(StandAloneTest, LPOPCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    auto ret = wrapper.lpop("not-exists-key", 10).get().getValue();
    ASSERT_EQ(0, ret.size());
    ASSERT_EQ(5, wrapper.rpush("mylist", {"one", "two", "three", "four", "five"}).get().getValue());
    ASSERT_EQ("one", *wrapper.lpop("mylist").get().getValue());
    auto ret2 = wrapper.lpop("mylist", 2).get().getValue();
    ASSERT_EQ(2, ret2.size());
    ASSERT_EQ("two", ret2[0]);
    ASSERT_EQ("three", ret2[1]);
}

TEST_F(StandAloneTest, LPUSHCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.lpush("mylist", {"World"}).get().getValue());
    ASSERT_EQ(2, wrapper.lpush("mylist", {"Hello"}).get().getValue());
    auto ret = wrapper.lrange("mylist", 0, -1).get().getValue();
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("Hello", ret[0]);
    ASSERT_EQ("World", ret[1]);
}

TEST_F(StandAloneTest, LPUSHXCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(0, wrapper.lpushx("mylist", {"World"}).get().getValue());
    ASSERT_EQ(1, wrapper.lpush("mylist", {"World"}).get().getValue());
    ASSERT_EQ(2, wrapper.lpushx("mylist", {"Hello"}).get().getValue());
    auto ret = wrapper.lrange("mylist", 0, -1).get().getValue();
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("Hello", ret[0]);
    ASSERT_EQ("World", ret[1]);
}

TEST_F(StandAloneTest, LREMCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(4, wrapper.rpush("mylist", {"hello", "hello", "foo", "hello"}).get().getValue());
    ASSERT_EQ(2, wrapper.lrem("mylist", -2, "hello").get().getValue());
}

TEST_F(StandAloneTest, LSETCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.rpush("mylist", {"one", "two", "three"}).get().getValue());
    ASSERT_EQ("OK", wrapper.lset("mylist", 0, "four").get().getValue());
    ASSERT_EQ("OK", wrapper.lset("mylist", -2, "five").get().getValue());
    auto ret = wrapper.lrange("mylist", 0, -1).get().getValue();
    ASSERT_EQ(3, ret.size());
    ASSERT_EQ("four", ret[0]);
    ASSERT_EQ("five", ret[1]);
    ASSERT_EQ("three", ret[2]);
}

TEST_F(StandAloneTest, LTRIMCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.rpush("mylist", {"one", "two", "three"}).get().getValue());
    ASSERT_EQ("OK", wrapper.ltrim("mylist", 1, -1).get().getValue());
    auto ret = wrapper.lrange("mylist", 0, -1).get().getValue();
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("two", ret[0]);
    ASSERT_EQ("three", ret[1]);
}

TEST_F(StandAloneTest, RPOPCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(5, wrapper.rpush("mylist", {"one", "two", "three", "four", "five"}).get().getValue());
    ASSERT_EQ("five", *wrapper.rpop("mylist").get().getValue());
    auto ret2 = wrapper.rpop("mylist", 2).get().getValue();
    ASSERT_EQ(2, ret2.size());
    ASSERT_EQ("four", ret2[0]);
    ASSERT_EQ("three", ret2[1]);
}

TEST_F(StandAloneTest, LMOVECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.rpush("mylist", {"one", "two", "three"}).get().getValue());
    ASSERT_EQ("three", *wrapper.lmove("mylist", "myotherlist", ListDirection::RIGHT, ListDirection::LEFT).get().getValue());
    ASSERT_EQ("one", *wrapper.lmove("mylist", "myotherlist", ListDirection::LEFT, ListDirection::RIGHT).get().getValue());
    ASSERT_EQ(1, wrapper.llen("mylist").get().getValue());
    auto ret = wrapper.lrange("myotherlist", 0, -1).get().getValue();
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("three", ret[0]);
    ASSERT_EQ("one", ret[1]);
}

TEST_F(StandAloneTest, BLMOVECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(3, wrapper.rpush("mylist", {"one", "two", "three"}).get().getValue());
    ASSERT_EQ("three", *wrapper.blmove("mylist", "myotherlist", ListDirection::RIGHT, ListDirection::LEFT, 0).get().getValue());
    ASSERT_EQ("one", *wrapper.blmove("mylist", "myotherlist", ListDirection::LEFT, ListDirection::RIGHT, 0).get().getValue());
    ASSERT_EQ(1, wrapper.llen("mylist").get().getValue());
    auto ret = wrapper.lrange("myotherlist", 0, -1).get().getValue();
    ASSERT_EQ(2, ret.size());
    ASSERT_EQ("three", ret[0]);
    ASSERT_EQ("one", ret[1]);
}