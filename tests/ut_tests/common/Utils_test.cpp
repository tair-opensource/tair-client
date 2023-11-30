/* Copyright (C) Alibaba Inc - All Rights Reserved
 *
 * Permission is hereby closed to people, company or entities that obtain the
 * term of use of this software and associated documentation files (the
 * "Software") without granted by Alibaba Inc develop team.
 * The use of copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software or use in other system is strictly prohibited.
 * Proprietary and confidential
 *
 */
#include "gtest/gtest.h"

#include "common/ClockTime.hpp"
#include "common/DateTime.hpp"
#include "common/StringUtil.hpp"
#include "common/SystemUtil.hpp"

using tair::common::ClockTime;
using tair::common::DateTime;
using tair::common::StringUtil;
using tair::common::SystemUtil;

TEST(STRING_TEST, UPPER_TEST) {
    std::string str;
    StringUtil::toUpper(str);
    ASSERT_EQ("", str);
    str = "abc";
    StringUtil::toUpper(str);
    ASSERT_EQ("ABC", str);
    str = "aBC";
    StringUtil::toUpper(str);
    ASSERT_EQ("ABC", str);
    str = "ABc";
    StringUtil::toUpper(str);
    ASSERT_EQ("ABC", str);
    str = "aBc";
    StringUtil::toUpper(str);
    ASSERT_EQ("ABC", str);
    str = "AbC";
    StringUtil::toUpper(str);
    ASSERT_EQ("ABC", str);
    str = "AbC__123";
    StringUtil::toUpper(str);
    ASSERT_EQ("ABC__123", str);
}

TEST(STRING_TEST, LOWER_TEST) {
    std::string str;
    StringUtil::toLower(str);
    ASSERT_EQ("", str);
    str = "ABC";
    StringUtil::toLower(str);
    ASSERT_EQ("abc", str);
    str = "aBC";
    StringUtil::toLower(str);
    ASSERT_EQ("abc", str);
    str = "ABc";
    StringUtil::toLower(str);
    ASSERT_EQ("abc", str);
    str = "aBc";
    StringUtil::toLower(str);
    ASSERT_EQ("abc", str);
    str = "AbC";
    StringUtil::toLower(str);
    ASSERT_EQ("abc", str);
    str = "AbC__123";
    StringUtil::toLower(str);
    ASSERT_EQ("abc__123", str);
}

TEST(STRING_TEST, EQUALS_IGNORE_CASE_TEST) {
    ASSERT_TRUE(StringUtil::equalsNoCase("", ""));
    ASSERT_TRUE(StringUtil::equalsNoCase("Abc", "abc"));
    ASSERT_TRUE(StringUtil::equalsNoCase("Hello", "heLLo"));
    ASSERT_TRUE(StringUtil::equalsNoCase("ApPle", "appLE"));
    ASSERT_TRUE(StringUtil::equalsNoCase("asdfgh", "asDfgh"));
    ASSERT_TRUE(StringUtil::equalsNoCase("ZxcVBnm", "zXCvbNm"));
    ASSERT_FALSE(StringUtil::equalsNoCase("asdfg", "zxcvn"));
}

TEST(STRING_TEST, START_WITH_TEST) {
    ASSERT_TRUE(StringUtil::startWith("ABCDEFG", "ABC"));
    ASSERT_TRUE(StringUtil::startWith("ZXCVBN", "Z"));
    ASSERT_TRUE(StringUtil::startWith("QWERTY", "QWERTY"));
    ASSERT_FALSE(StringUtil::startWith("QWERTY", "qwer"));
    ASSERT_FALSE(StringUtil::startWith("QWERTY", "12345"));

    ASSERT_TRUE(StringUtil::startWith("ABCDEFG", std::string("ABC")));
    ASSERT_TRUE(StringUtil::startWith("ZXCVBN", std::string("Z")));
    ASSERT_TRUE(StringUtil::startWith("QWERTY", std::string("QWERTY")));
    ASSERT_FALSE(StringUtil::startWith("QWERTY", std::string("qwer")));
    ASSERT_FALSE(StringUtil::startWith("QWERTY", std::string("12345")));
}

TEST(STRING_TEST, START_WITH_IGNORE_CASE_TEST) {
    ASSERT_TRUE(StringUtil::startWithNoCase("ABCDEFG", "ABC"));
    ASSERT_TRUE(StringUtil::startWithNoCase("ABCDEFG", "abC"));
    ASSERT_TRUE(StringUtil::startWithNoCase("ZXCVBN", "Z"));
    ASSERT_TRUE(StringUtil::startWithNoCase("ZXCVBN", "zx"));
    ASSERT_TRUE(StringUtil::startWithNoCase("QWERTY", "QWERTY"));
    ASSERT_TRUE(StringUtil::startWithNoCase("QWERTY", "qwer"));
    ASSERT_FALSE(StringUtil::startWithNoCase("QWERTY", "12345"));

    ASSERT_TRUE(StringUtil::startWithNoCase("ABCDEFG", std::string("ABC")));
    ASSERT_TRUE(StringUtil::startWithNoCase("ABCDEFG", std::string("abc")));
    ASSERT_TRUE(StringUtil::startWithNoCase("ZXCVBN", std::string("Z")));
    ASSERT_TRUE(StringUtil::startWithNoCase("ZXCVBN", std::string("zx")));
    ASSERT_TRUE(StringUtil::startWithNoCase("QWERTY", std::string("QWERTY")));
    ASSERT_TRUE(StringUtil::startWithNoCase("QWERTY", std::string("qwer")));
    ASSERT_FALSE(StringUtil::startWithNoCase("QWERTY", std::string("12345")));
}

TEST(STRING_TEST, SPLIT_TEST) {
    auto result = StringUtil::split("abc def ghi", ' ');
    ASSERT_EQ(3U, result.size());
    ASSERT_EQ("abc", result[0]);
    ASSERT_EQ("def", result[1]);
    ASSERT_EQ("ghi", result[2]);

    result = StringUtil::split(" def", ' ');
    ASSERT_EQ(2U, result.size());
    ASSERT_EQ("", result[0]);
    ASSERT_EQ("def", result[1]);

    result = StringUtil::split(":0", ':');
    ASSERT_EQ(2U, result.size());
    ASSERT_EQ("", result[0]);
    ASSERT_EQ("0", result[1]);
}

TEST(STRING_TEST, TRIM_TEST) {
    std::string str;
    StringUtil::trim(str);
    ASSERT_EQ("", str);

    str = "\t\t\rabcdefg\n\n";
    StringUtil::trim(str);
    ASSERT_EQ("abcdefg", str);

    str = "  \t\t\rabcdefg\n\n";
    StringUtil::trim(str);
    ASSERT_EQ("abcdefg", str);

    str = "  \t\t\rabc\ndefg\n\n";
    StringUtil::trim(str);
    ASSERT_EQ("abc\ndefg", str);

    str = "abcdefgbbbbbc";
    StringUtil::trim(str, "abc");
    ASSERT_EQ("defg", str);
}

