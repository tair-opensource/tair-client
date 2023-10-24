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
#include "fmt/format.h"
#include "gtest/gtest.h"

TEST(FORMAT_TEST, ONLY_TEST) {
    char buf[10];
    char *end = fmt::format_to(buf, "{}", 42);
    ASSERT_EQ('4', buf[0]);
    ASSERT_EQ('2', buf[1]);
    ASSERT_EQ(end, &buf[2]);
    ASSERT_EQ(2U, end - buf);

    ASSERT_EQ("REDIS0001", fmt::format("REDIS{:04d}", 1));
}

TEST(FORMAT_TEST, STRING) {
    std::string decoded;
    int runlen = 10;
    decoded = fmt::format("{}Z:{} ", decoded, runlen);
    ASSERT_EQ(decoded, "Z:10 ");
    decoded = fmt::format("{}v:{},{} ", decoded, runlen, runlen);
    ASSERT_EQ(decoded, "Z:10 v:10,10 ");

    ASSERT_EQ(5, fmt::formatted_size("{}", 12345));
}
