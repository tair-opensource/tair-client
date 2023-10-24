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

TEST_F(StandAloneTest, XADDCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
}

TEST_F(StandAloneTest, XLENCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    ASSERT_EQ(1, wrapper.xlen("mystream").get().getValue());
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    ASSERT_EQ(2, wrapper.xlen("mystream").get().getValue());
}

TEST_F(StandAloneTest, XDELCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    auto id1 = wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue();
    auto id2 = wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue();
    ASSERT_EQ(2, wrapper.xlen("mystream").get().getValue());
    ASSERT_EQ(2, wrapper.xdel("mystream", {id1, id2}).get().getValue());
}

TEST_F(StandAloneTest, XGROUPCREATECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    ASSERT_EQ("OK", wrapper.xgroupCreate("mystream", "mygroup", "$").get().getValue());
}

TEST_F(StandAloneTest, XGROUPCREATECONSUMERCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    ASSERT_EQ("OK", wrapper.xgroupCreate("mystream", "mygroup", "$").get().getValue());
    ASSERT_EQ(1, wrapper.xgroupCreateConsumer("mystream", "mygroup", "myconsumer").get().getValue());
}

TEST_F(StandAloneTest, XGROUPDELCONSUMERCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    ASSERT_EQ("OK", wrapper.xgroupCreate("mystream", "mygroup", "$").get().getValue());
    ASSERT_EQ(1, wrapper.xgroupCreateConsumer("mystream", "mygroup", "myconsumer").get().getValue());
    ASSERT_EQ(0, wrapper.xgroupDelConsumer("mystream", "mygroup", "myconsumer").get().getValue());
}

TEST_F(StandAloneTest, XGROUPDESTROYCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    ASSERT_EQ("OK", wrapper.xgroupCreate("mystream", "mygroup", "$").get().getValue());
    ASSERT_EQ(1, wrapper.xgroupDestroy("mystream", "mygroup").get().getValue());
}

TEST_F(StandAloneTest, XRANGECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    auto ret = wrapper.xrange("mystream", "-", "+").get().getValue();
    ASSERT_EQ(1, ret.size());
    ASSERT_NE("", ret[0].id);
    ASSERT_EQ(4, ret[0].values.size());
    ASSERT_EQ("sensor-id", ret[0].values[0]);
    ASSERT_EQ("1234", ret[0].values[1]);
    ASSERT_EQ("temperature", ret[0].values[2]);
    ASSERT_EQ("19.8", ret[0].values[3]);
}

TEST_F(StandAloneTest, XREVRANGECOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    auto ret = wrapper.xrevrange("mystream", "+", "-").get().getValue();
    ASSERT_EQ(1, ret.size());
    ASSERT_NE("", ret[0].id);
    ASSERT_EQ(4, ret[0].values.size());
    ASSERT_EQ("sensor-id", ret[0].values[0]);
    ASSERT_EQ("1234", ret[0].values[1]);
    ASSERT_EQ("temperature", ret[0].values[2]);
    ASSERT_EQ("19.8", ret[0].values[3]);
}

TEST_F(StandAloneTest, XREADCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    auto ret = wrapper.xread(1, {"mystream"}, {"0"}).get().getValue();
    ASSERT_EQ(1, ret.size());
    ASSERT_EQ("mystream", ret[0].streamname);
    ASSERT_EQ(1, ret[0].infos.size());
    ASSERT_NE("", ret[0].infos[0].id);
    ASSERT_EQ("sensor-id", ret[0].infos[0].values[0]);
    ASSERT_EQ("1234", ret[0].infos[0].values[1]);
    ASSERT_EQ("temperature", ret[0].infos[0].values[2]);
    ASSERT_EQ("19.8", ret[0].infos[0].values[3]);
}

TEST_F(StandAloneTest, XREADGROUPCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    ASSERT_EQ("OK", wrapper.xgroupCreate("mystream", "mygroup", "$").get().getValue());
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    auto ret = wrapper.xreadgroup("mygroup", "alice", {"mystream"}, {">"}).get().getValue();
    ASSERT_EQ(1, ret.size());
    ASSERT_EQ("mystream", ret[0].streamname);
    ASSERT_EQ(1, ret[0].infos.size());
    ASSERT_NE("", ret[0].infos[0].id);
    ASSERT_EQ("sensor-id", ret[0].infos[0].values[0]);
    ASSERT_EQ("1234", ret[0].infos[0].values[1]);
    ASSERT_EQ("temperature", ret[0].infos[0].values[2]);
    ASSERT_EQ("19.8", ret[0].infos[0].values[3]);
}

TEST_F(StandAloneTest, XACKCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    ASSERT_EQ("OK", wrapper.xgroupCreate("mystream", "mygroup", "$").get().getValue());
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    auto ret = wrapper.xreadgroup("mygroup", "alice", {"mystream"}, {">"}).get().getValue();
    ASSERT_EQ(1, wrapper.xack("mystream", "mygroup", {ret[0].infos[0].id}).get().getValue());
}

TEST_F(StandAloneTest, XPENDINGCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    ASSERT_EQ("OK", wrapper.xgroupCreate("mystream", "mygroup", "$").get().getValue());
    ASSERT_NE("", wrapper.xadd("mystream", "*", {"sensor-id", "1234", "temperature", "19.8"}).get().getValue());
    wrapper.xreadgroup("mygroup", "alice", {"mystream"}, {">"}).get().getValue();
    auto ret = wrapper.xpending("mystream", "mygroup").get().getValue();
    ASSERT_EQ(1, ret.pel_number);
    ASSERT_NE("", ret.startid);
    ASSERT_NE("", ret.endid);
    ASSERT_EQ(1, ret.messages.size());
    ASSERT_EQ("alice", ret.messages[0].first);
    ASSERT_EQ("1", ret.messages[0].second);
}
