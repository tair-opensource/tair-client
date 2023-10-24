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

#include "common/statistics/LatencyStatistics.hpp"

using tair::common::LatencyMetric;

//     * 0          < 1us   0
//     * 1          < 2us   1
//     * 2          < 4us   2~3
//     * 3          < 8us   4~7
//     * 4          < 16us  8~15
//     * 5          < 32us  16~31
//     * 6          < 64us  32~63
//     ......
//     * 25         < 33554432us (~33s)
//     * 26         >= 33554432us (33s)

TEST(LATENCY_METRIC_CALC_BUCKET_TEST, ONLY_TEST) {
    ASSERT_EQ(0, LatencyMetric::calcBucket(0));
    ASSERT_EQ(1, LatencyMetric::calcBucket(1));

    ASSERT_EQ(2, LatencyMetric::calcBucket(2));
    ASSERT_EQ(2, LatencyMetric::calcBucket(3));

    ASSERT_EQ(3, LatencyMetric::calcBucket(4));
    ASSERT_EQ(3, LatencyMetric::calcBucket(5));
    ASSERT_EQ(3, LatencyMetric::calcBucket(6));
    ASSERT_EQ(3, LatencyMetric::calcBucket(7));

    ASSERT_EQ(4, LatencyMetric::calcBucket(8));
    ASSERT_EQ(4, LatencyMetric::calcBucket(15));

    ASSERT_EQ(5, LatencyMetric::calcBucket(16));
    ASSERT_EQ(5, LatencyMetric::calcBucket(31));

    ASSERT_EQ(6, LatencyMetric::calcBucket(32));
    ASSERT_EQ(6, LatencyMetric::calcBucket(63));

    ASSERT_EQ(25, LatencyMetric::calcBucket(33554432 - 1));
    ASSERT_EQ(26, LatencyMetric::calcBucket(33554432));
    ASSERT_EQ(26, LatencyMetric::calcBucket(33554432 + 1));
}
