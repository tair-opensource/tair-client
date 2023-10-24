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
#pragma once

#include "common/statistics/AtomicStatistics.hpp"

namespace tair::network {

enum NetworkStatType : int {
    NETWORK_STATS_TOTAL_NET_GENERAL_BUFFER,
    NETWORK_STATS_TOTAL_NET_INPUT_BUFFER,
    NETWORK_STATS_TOTAL_NET_OUTPUT_BUFFER,
    NETWORK_STATS_TOTAL_NET_INPUT_BYTES,
    NETWORK_STATS_TOTAL_NET_OUTPUT_BYTES,
    NETWORK_STATS_TOTAL_MOVING_TCP_CONN_COUNT,
    NETWORK_STATS_COUNT,
};

using NetworkStatistics = common::AtomicStatistics<NetworkStatType, NETWORK_STATS_COUNT>;
using NetworkStatisticsHelper = common::AtomicStatisticsHelper<NetworkStatType, NetworkStatistics, NETWORK_STATS_COUNT>;

#define NETWORK_STAT_TOTAL_ADD_FUNC(NAME, TYPE)       \
    static inline void add##NAME(int64_t size) {      \
        NetworkStatisticsHelper::addStat(TYPE, size); \
    }

#define NETWORK_STAT_TOTAL_SUB_FUNC(NAME, TYPE)       \
    static inline void sub##NAME(int64_t size) {      \
        NetworkStatisticsHelper::subStat(TYPE, size); \
    }

#define NETWORK_STAT_TOTAL_GET_FUNC(NAME, TYPE)        \
    static inline int64_t get##NAME() {                \
        return NetworkStatisticsHelper::getStat(TYPE); \
    }

#define NETWORK_STAT_TOTAL_GET_OPS_FUNC(NAME, TYPE)       \
    static inline int64_t get##NAME##Ops() {              \
        return NetworkStatisticsHelper::getStatOps(TYPE); \
    }

#define NETWORK_STAT_TOTAL_ADD_GET_FUNC(NAME, TYPE) \
    NETWORK_STAT_TOTAL_ADD_FUNC(NAME, TYPE)         \
    NETWORK_STAT_TOTAL_GET_FUNC(NAME, TYPE)         \
    NETWORK_STAT_TOTAL_GET_OPS_FUNC(NAME, TYPE)

#define NETWORK_STAT_TOTAL_ADD_SUB_FUNC(NAME, TYPE) \
    NETWORK_STAT_TOTAL_SUB_FUNC(NAME, TYPE)         \
    NETWORK_STAT_TOTAL_ADD_FUNC(NAME, TYPE)         \
    NETWORK_STAT_TOTAL_GET_FUNC(NAME, TYPE)

class NetworkStat {
public:
    static inline void calcStatistics() {
        NetworkStatisticsHelper::calcStatistics();
    }

    NETWORK_STAT_TOTAL_ADD_SUB_FUNC(GeneralBufferSize, NETWORK_STATS_TOTAL_NET_GENERAL_BUFFER)
    NETWORK_STAT_TOTAL_ADD_SUB_FUNC(NetInputBufferSize, NETWORK_STATS_TOTAL_NET_INPUT_BUFFER)
    NETWORK_STAT_TOTAL_ADD_SUB_FUNC(NetOutputBufferSize, NETWORK_STATS_TOTAL_NET_OUTPUT_BUFFER)

    NETWORK_STAT_TOTAL_ADD_GET_FUNC(NetInputBytes, NETWORK_STATS_TOTAL_NET_INPUT_BYTES)
    NETWORK_STAT_TOTAL_ADD_GET_FUNC(NetOutputBytes, NETWORK_STATS_TOTAL_NET_OUTPUT_BYTES)

    NETWORK_STAT_TOTAL_ADD_SUB_FUNC(MovingTcpConnCount, NETWORK_STATS_TOTAL_MOVING_TCP_CONN_COUNT)
};

} // namespace tair::network
