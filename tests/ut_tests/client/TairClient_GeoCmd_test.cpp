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

#include <optional>
#include <tuple>
#include <utility>

#include "client/params/ParamsAll.hpp"
#include "TairClient_Standalone_Server.hpp"

using tair::client::GeoUnit;
using tair::client::GeoPosResult;

TEST_F(StandAloneTest, GEOADDCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    std::vector<std::tuple<double, double, std::string>> members;
    members.emplace_back(13.361389, 38.115556, "Palermo");
    members.emplace_back(15.087269, 37.502669, "Catania");
    ASSERT_EQ(2, wrapper.geoadd("Sicily", members).get().getValue());
}

TEST_F(StandAloneTest, GEODISTCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    std::vector<std::tuple<double, double, std::string>> members;
    members.emplace_back(13.361389, 38.115556, "Palermo");
    members.emplace_back(15.087269, 37.502669, "Catania");
    ASSERT_EQ(2, wrapper.geoadd("Sicily", members).get().getValue());
    ASSERT_EQ("166274.1516", *wrapper.geodist("Sicily", "Palermo", "Catania", GeoUnit::M).get().getValue());
    ASSERT_EQ(nullptr, wrapper.geodist("Sicily", "not-exist", "not-exist", GeoUnit::M).get().getValue());
}

TEST_F(StandAloneTest, GEOHASHCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    std::vector<std::tuple<double, double, std::string>> members;
    members.emplace_back(13.361389, 38.115556, "Palermo");
    members.emplace_back(15.087269, 37.502669, "Catania");
    ASSERT_EQ(2, wrapper.geoadd("Sicily", members).get().getValue());
    auto ret = wrapper.geohash("Sicily", {"Palermo", "Catania"}).get().getValue();
    ASSERT_EQ("sqc8b49rny0", *ret[0]);
    ASSERT_EQ("sqdtr74hyu0", *ret[1]);
}

TEST_F(StandAloneTest, GEOPOSCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    std::vector<std::tuple<double, double, std::string>> members;
    members.emplace_back(13.361389, 38.115556, "Palermo");
    members.emplace_back(15.087269, 37.502669, "Catania");
    ASSERT_EQ(2, wrapper.geoadd("Sicily", members).get().getValue());
    auto ret = wrapper.geopos("Sicily", {"Palermo", "Catania", "not-exists"}).get().getValue();
    ASSERT_EQ("13.36138933897018433", ret[0]->first);
    ASSERT_EQ("38.11555639549629859", ret[0]->second);
    ASSERT_EQ("15.08726745843887329", ret[1]->first);
    ASSERT_EQ("37.50266842333162032", ret[1]->second);
    ASSERT_EQ(std::nullopt, ret[2]);
}

TEST_F(StandAloneTest, GEORADIUSCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    std::vector<std::tuple<double, double, std::string>> members;
    members.emplace_back(13.361389, 38.115556, "Palermo");
    members.emplace_back(15.087269, 37.502669, "Catania");
    ASSERT_EQ(2, wrapper.geoadd("Sicily", members).get().getValue());
    auto ret = wrapper.georadius("Sicily", 15, 37, 200, GeoUnit::KM).get().getValue();
    ASSERT_EQ("Palermo", ret[0]);
    ASSERT_EQ("Catania", ret[1]);
}

TEST_F(StandAloneTest, GEORADIUSBYMEMBERCOMMAND) {
    auto wrapper = StandAloneTest::client->getFutureWrapper();
    std::vector<std::tuple<double, double, std::string>> members;
    members.emplace_back(13.361389, 38.115556, "Palermo");
    members.emplace_back(15.087269, 37.502669, "Catania");
    members.emplace_back(13.583333, 37.316667, "Agrigento");
    ASSERT_EQ(3, wrapper.geoadd("Sicily", members).get().getValue());
    auto ret = wrapper.georadiusbymember("Sicily", "Agrigento", 100, GeoUnit::KM).get().getValue();
    ASSERT_EQ("Agrigento", ret[0]);
    ASSERT_EQ("Palermo", ret[1]);
}