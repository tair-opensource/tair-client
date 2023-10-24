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

using tair::client::BitPositonParams;
using tair::client::BitOperation;
using tair::client::SetParams;
using tair::client::GetExParams;

TEST_F(StandAloneTest, GETCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(nullptr, wrapper.get("not-exists-key").get().getValue());
}

TEST_F(StandAloneTest, BITFIELD_COMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    // BITFIELD mykey
    auto r = wrapper.bitfield("mykey", {"INCRBY"}).get();
    ASSERT_FALSE(r.isSuccess());
    ASSERT_EQ("ERR syntax error", r.getErr());

    // BITFIELD mykey INCRBY i5 100 1 GET u4 0
    auto r1 = wrapper.bitfield("mykey", {"INCRBY", "i5", "100", "1", "GET", "u4", "0"}).get();
    ASSERT_TRUE(r1.isSuccess());
    ASSERT_EQ(2, r1.getValue().size());
    ASSERT_EQ(1, *r1.getValue()[0]);
    ASSERT_EQ(0, *r1.getValue()[1]);

    // BITFIELD mykey incrby u2 100 1 OVERFLOW SAT incrby u2 102 1
    for (int i = 0; i < 3; ++i) {
        wrapper.bitfield("mykey", {"INCRBY", "u2", "100", "1", "OVERFLOW", "SAT", "INCRBY", "u2", "102", "1"}).get();
    }
    auto r2 = wrapper.bitfield("mykey", {"INCRBY", "u2", "100", "1", "OVERFLOW", "SAT", "INCRBY", "u2", "102", "1"}).get();
    ASSERT_TRUE(r2.isSuccess());
    ASSERT_EQ(2, r2.getValue().size());
    ASSERT_EQ(0, *r2.getValue()[0]);
    ASSERT_EQ(3, *r2.getValue()[1]);

    // BITFIELD mykey OVERFLOW FAIL incrby u2 102 1
    auto r3 = wrapper.bitfield("mykey", {"OVERFLOW", "FAIL", "INCRBY", "u2", "102", "1"}).get();
    ASSERT_TRUE(r3.isSuccess());
    ASSERT_EQ(1, r3.getValue().size());
    ASSERT_EQ(nullptr, r3.getValue()[0]);
}

TEST_F(StandAloneTest, APPEND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(5, wrapper.append("mykey", "hello").get().getValue());
    ASSERT_EQ(11, wrapper.append("mykey", " world").get().getValue());
    ASSERT_EQ("hello world", *(wrapper.get("mykey").get().getValue()));
}

TEST_F(StandAloneTest, BITCOUNT) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("mykey", "foobar").get().getValue());
    ASSERT_EQ(26, wrapper.bitcount("mykey").get().getValue());
    BitPositonParams params;
    ASSERT_EQ(4, wrapper.bitcount("mykey", params.start(0).end(0)).get().getValue());
    ASSERT_EQ(6, wrapper.bitcount("mykey", params.start(1).end(1)).get().getValue());
    ASSERT_EQ(17, wrapper.bitcount("mykey", params.start(5).end(30).bit()).get().getValue());
}

TEST_F(StandAloneTest, BITGETSET) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(0, wrapper.setbit("mykey", 7, 1).get().getValue());
    ASSERT_EQ(0, wrapper.getbit("mykey", 0).get().getValue());
    ASSERT_EQ(1, wrapper.getbit("mykey", 7).get().getValue());
    ASSERT_EQ(0, wrapper.getbit("mykey", 100).get().getValue());
    ASSERT_EQ(1, wrapper.setbit("mykey", 7, 0).get().getValue());
}

TEST_F(StandAloneTest, BITOP) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("key1", "foobar").get().getValue());
    ASSERT_EQ("OK", wrapper.set("key2", "abcdef").get().getValue());
    ASSERT_EQ(6, wrapper.bitop(BitOperation::AND, {"dest", "key1", "key2"}).get().getValue());
}

TEST_F(StandAloneTest, INCRDECR) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("mykey", "10").get().getValue());
    ASSERT_EQ(9, wrapper.decr("mykey").get().getValue());
    ASSERT_EQ(11, wrapper.incrby("mykey", 2).get().getValue());
    ASSERT_EQ(9, wrapper.decrby("mykey", 2).get().getValue());
}

TEST_F(StandAloneTest, MSETGET) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.mset({"k1", "v1", "k2", "v2", "k3", "v3"}).get().getValue());
    auto values = wrapper.mget({"k1", "k2", "k3"}).get().getValue();
    ASSERT_EQ(3, values.size());
    ASSERT_EQ("v1", *values[0]);
    ASSERT_EQ("v2", *values[1]);
    ASSERT_EQ("v3", *values[2]);
}

TEST_F(StandAloneTest, GETDEL) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("mykey", "Hello").get().getValue());
    ASSERT_EQ("Hello", *wrapper.getdel("mykey").get().getValue());
    ASSERT_EQ(nullptr, wrapper.get("mykey").get().getValue());
}

TEST_F(StandAloneTest, BITPOS) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("mykey", "\\xff\\xf0\\x00").get().getValue());
    ASSERT_EQ(0, wrapper.bitpos("mykey", 0).get().getValue());
    ASSERT_EQ("OK", wrapper.set("mykey", "\\x00\\xff\\xf0").get().getValue());
    BitPositonParams b1;
    ASSERT_EQ(1, wrapper.bitpos("mykey", 1, b1.start(0)).get().getValue());
    BitPositonParams b2;
    ASSERT_EQ(18, wrapper.bitpos("mykey", 1, b2.start(2)).get().getValue());
    BitPositonParams b3;
    ASSERT_EQ(18, wrapper.bitpos("mykey", 1, b3.start(2).end(-1).byte()).get().getValue());
    BitPositonParams b4;
    ASSERT_EQ(9, wrapper.bitpos("mykey", 1, b4.start(7).end(15).bit()).get().getValue());
}

TEST_F(StandAloneTest, GETRANGE) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("mykey", "This is a string").get().getValue());
    ASSERT_EQ("This", wrapper.getrange("mykey", 0, 3).get().getValue());
    ASSERT_EQ("ing", wrapper.getrange("mykey", -3, -1).get().getValue());
    ASSERT_EQ("This is a string", wrapper.getrange("mykey", 0, -1).get().getValue());
    ASSERT_EQ("string", wrapper.getrange("mykey", 10, 100).get().getValue());
}

TEST_F(StandAloneTest, GETSET) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.incr("mycounter").get().getValue());
    ASSERT_EQ("1", *wrapper.getset("mycounter", "0").get().getValue());
    ASSERT_EQ("0", *(wrapper.get("mycounter").get().getValue()));
}

TEST_F(StandAloneTest, SETPARAMS) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    SetParams p1;
    ASSERT_EQ("OK", wrapper.set("mykey", "Hello", p1.nx()).get().getValue());
    SetParams p2;
    ASSERT_EQ("OK", wrapper.set("mykey", "World", p2.xx()).get().getValue());
}

TEST_F(StandAloneTest, INCRBYFLOAT) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("mykey", "10.50").get().getValue());
    ASSERT_DOUBLE_EQ(10.6, std::stod(wrapper.incrbyfloat("mykey", 0.1).get().getValue()));
    ASSERT_DOUBLE_EQ(5.6, std::stod(wrapper.incrbyfloat("mykey", -5).get().getValue()));
    ASSERT_EQ("OK", wrapper.set("mykey", "5.0e3").get().getValue());
    ASSERT_EQ("5200", wrapper.incrbyfloat("mykey", 2.0e2).get().getValue());
}

TEST_F(StandAloneTest, MSETNX) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.msetnx({"key1", "Hello", "key2", "there"}).get().getValue());
    ASSERT_EQ(0, wrapper.msetnx({"key2", "new", "key3", "world"}).get().getValue());
    auto values = wrapper.mget({"key1", "key2", "key3"}).get().getValue();
    ASSERT_EQ(3, values.size());
    ASSERT_EQ("Hello", *values[0]);
    ASSERT_EQ("there", *values[1]);
    ASSERT_EQ(nullptr, values[2]);
}

TEST_F(StandAloneTest, PSETEX) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.psetex("mykey", 1000, "Hello").get().getValue());
    ASSERT_GE(1000, wrapper.pttl("mykey").get().getValue());
    ASSERT_EQ("Hello", *wrapper.get("mykey").get().getValue());
}

TEST_F(StandAloneTest, SETEX) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.setex("mykey", 10, "Hello").get().getValue());
    ASSERT_GE(10, wrapper.ttl("mykey").get().getValue());
    ASSERT_EQ("Hello", *wrapper.get("mykey").get().getValue());
}

TEST_F(StandAloneTest, SETNX) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ(1, wrapper.setnx("mykey", "Hello").get().getValue());
    ASSERT_EQ(0, wrapper.setnx("mykey", "World").get().getValue());
    ASSERT_EQ("Hello", *wrapper.get("mykey").get().getValue());
}

TEST_F(StandAloneTest, SETRANGE) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("key", "Hello World").get().getValue());
    ASSERT_EQ(11, wrapper.setrange("key", 6, "Redis").get().getValue());
    ASSERT_EQ("Hello Redis", *wrapper.get("key").get().getValue());
    ASSERT_EQ(11, wrapper.strlen("key").get().getValue());
}

TEST_F(StandAloneTest, GETEX) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_EQ("OK", wrapper.set("mykey", "Hello").get().getValue());
    GetExParams p1;
    ASSERT_EQ("Hello", *wrapper.getex("mykey", p1).get().getValue());
    ASSERT_EQ(-1, wrapper.ttl("mykey").get().getValue());
    GetExParams p2;
    ASSERT_EQ("Hello", *wrapper.getex("mykey", p2.ex(60)).get().getValue());
    ASSERT_GE(60, wrapper.ttl("mykey").get().getValue());
}
