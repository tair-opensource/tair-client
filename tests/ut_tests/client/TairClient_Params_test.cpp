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

#include "client/params/ParamsAll.hpp"

using tair::client::ExpireParams;
using tair::client::RestoreParams;
using tair::client::SortParams;
using tair::client::BitPositonParams;
using tair::client::CopyParams;
using tair::client::GetExParams;
using tair::client::SetParams;
using tair::client::ScanParams;
using tair::client::ZAddParams;
using tair::client::ZInterUnionParams;
using tair::client::ZRangeParams;
using tair::client::ZRemRangeOption;
using tair::client::zremRangeOptionToString;

TEST(PARAMS, EXPIREPARAMS) {
    std::vector<std::string> argv;
    ExpireParams params;
    params.gt().nx();
    params.addParamsToArgv(argv);
    ASSERT_EQ(2, argv.size());
    ASSERT_EQ("NX", argv[0]);
    ASSERT_EQ("GT", argv[1]);
}

TEST(PARAMS, RESTOREPARRAMS) {
    std::vector<std::string> argv;
    RestoreParams params;
    params.replace().absttl().idletime(1000);
    params.addParamsToArgv(argv);
    ASSERT_EQ(4, argv.size());
    ASSERT_EQ("REPLACE", argv[0]);
    ASSERT_EQ("ABSTTL", argv[1]);
    ASSERT_EQ("IDLETIME", argv[2]);
    ASSERT_EQ("1000", argv[3]);
}

TEST(PARAMS, SORTPARAMS) {
    std::vector<std::string> argv;
    SortParams params;
    std::vector<std::string> get_pattern {"get_1", "get_2"};
    params.by("pattern").limit(100, 200).asc().get(get_pattern);
    params.addParamsToArgv(argv);
    ASSERT_EQ(10, argv.size());
    ASSERT_EQ("BY", argv[0]);
    ASSERT_EQ("pattern", argv[1]);
    ASSERT_EQ("LIMIT", argv[2]);
    ASSERT_EQ("100", argv[3]);
    ASSERT_EQ("200", argv[4]);
    ASSERT_EQ("GET", argv[5]);
    ASSERT_EQ("get_1", argv[6]);
    ASSERT_EQ("GET", argv[7]);
    ASSERT_EQ("get_2", argv[8]);
    ASSERT_EQ("ASC", argv[9]);
}

TEST(PARAMS, BITPOSITIONPARAMS) {
    std::vector<std::string> argv;
    BitPositonParams params;
    params.start(0).end(100).byte();
    params.addParamsToArgv(argv);
    ASSERT_EQ(3, argv.size());
    ASSERT_EQ("0", argv[0]);
    ASSERT_EQ("100", argv[1]);
    ASSERT_EQ("BYTE", argv[2]);
}

TEST(PARAMS, COPYPARAMS) {
    std::vector<std::string> argv;
    CopyParams params;
    params.db(10).replace();
    params.addParamsToArgv(argv);
    ASSERT_EQ(3, argv.size());
    ASSERT_EQ("DB", argv[0]);
    ASSERT_EQ("10", argv[1]);
    ASSERT_EQ("REPLACE", argv[2]);
}

TEST(PARAMS, GETEXPARAMS) {
    std::vector<std::string> argv;
    GetExParams params;
    params.ex(100).px(1000).exat(10000).persist();
    params.addParamsToArgv(argv);
    ASSERT_EQ(7, argv.size());
    ASSERT_EQ("PERSIST", argv[0]);
    ASSERT_EQ("EX", argv[1]);
    ASSERT_EQ("100", argv[2]);
    ASSERT_EQ("PX", argv[3]);
    ASSERT_EQ("1000", argv[4]);
    ASSERT_EQ("EXAT", argv[5]);
    ASSERT_EQ("10000", argv[6]);
}

TEST(PARAMS, SETPARAMS) {
    std::vector<std::string> argv;
    SetParams params;
    params.ex(100).px(1000).exat(10000).nx().xx().keepttl();
    params.addParamsToArgv(argv);
    ASSERT_EQ(9, argv.size());
    ASSERT_EQ("NX", argv[0]);
    ASSERT_EQ("XX", argv[1]);
    ASSERT_EQ("KEEPTTL", argv[2]);
    ASSERT_EQ("EX", argv[3]);
    ASSERT_EQ("100", argv[4]);
    ASSERT_EQ("PX", argv[5]);
    ASSERT_EQ("1000", argv[6]);
    ASSERT_EQ("EXAT", argv[7]);
    ASSERT_EQ("10000", argv[8]);
}

TEST(PARAMS, SCANPARAMS) {
    std::vector<std::string> argv;
    ScanParams params;
    params.match("*").count(10).type("hash");
    params.addParamsToArgv(argv);
    ASSERT_EQ(6, argv.size());
    ASSERT_EQ("MATCH", argv[0]);
    ASSERT_EQ("*", argv[1]);
    ASSERT_EQ("COUNT", argv[2]);
    ASSERT_EQ("10", argv[3]);
    ASSERT_EQ("TYPE", argv[4]);
    ASSERT_EQ("hash", argv[5]);
}

TEST(PARAMS, ZADDPARAMS) {
    std::vector<std::string> argv;
    ZAddParams params;
    params.nx().xx().gt().ch();
    params.addParamsToArgv(argv);
    ASSERT_EQ(4, argv.size());
    ASSERT_EQ("NX", argv[0]);
    ASSERT_EQ("XX", argv[1]);
    ASSERT_EQ("GT", argv[2]);
    ASSERT_EQ("CH", argv[3]);
}

TEST(PARAMS, ZINTERUNIONPARAMS) {
    std::vector<std::string> argv;
    ZInterUnionParams params;
    params.weights({1.0, 2.0, 3.0}).aggregate("max");
    params.addParamsToArgv(argv);
    ASSERT_EQ(6, argv.size());
    ASSERT_EQ("WEIGHTS", argv[0]);
    // With floating point types std::to_string may yield unexpected results as the number of significant
    // digits in the returned string can be zero, see https://en.cppreference.com/w/cpp/string/basic_string/to_string
    ASSERT_EQ("1.000000", argv[1]);
    ASSERT_EQ("2.000000", argv[2]);
    ASSERT_EQ("3.000000", argv[3]);
    ASSERT_EQ("AGGREGATE", argv[4]);
    ASSERT_EQ("max", argv[5]);
}

TEST(PARAMS, ZRANGEPARAMS) {
    std::vector<std::string> argv;
    ZRangeParams params;
    params.limit(0, 10).byscore().withscores().rev();
    params.addParamsToArgv(argv);
    ASSERT_EQ(6, argv.size());
    ASSERT_EQ("BYSCORE", argv[0]);
    ASSERT_EQ("REV", argv[1]);
    ASSERT_EQ("LIMIT", argv[2]);
    ASSERT_EQ("0", argv[3]);
    ASSERT_EQ("10", argv[4]);
    ASSERT_EQ("WITHSCORES", argv[5]);
}

TEST(PARAMS, ZREMRANGEOPTION) {
    std::vector<std::string> argv;
    ASSERT_EQ("zremrangeBYSCORE", "zremrange" + zremRangeOptionToString(ZRemRangeOption::BYSCORE));
    ASSERT_EQ("zremrangeBYRANK", "zremrange" + zremRangeOptionToString(ZRemRangeOption::BYRANK));
    ASSERT_EQ("zremrangeBYLEX", "zremrange" + zremRangeOptionToString(ZRemRangeOption::BYLEX));
}