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

#include <atomic>

#include "common/ClockTime.hpp"
#include "common/Noncopyable.hpp"

namespace tair::common {

template <typename TYPE, int stats_count>
class AtomicStatistics final : private Noncopyable {
public:
    AtomicStatistics() = default;
    ~AtomicStatistics() = default;

    inline void clear(int start_idx = 0, int end_index = INT32_MAX) {
        for (int i = start_idx; i < stats_count && i <= end_index; ++i) {
            stats_[i] = 0;
        }
    }

    inline void subStat(TYPE type, int64_t count) {
        stats_[type] -= count;
    }

    inline void addStat(TYPE type, int64_t count) {
        stats_[type] += count;
    }

    inline int64_t getStat(TYPE type) {
        return stats_[type];
    }

private:
    std::atomic<int64_t> stats_[stats_count] = {0};
};

constexpr static const int32_t ATOMIC_STATS_SAMPLES_COUNT = 16;
struct AtomicMetric : private Noncopyable {
    void clear() {
        last_sample_count = 0;
        for (int i = 0; i < ATOMIC_STATS_SAMPLES_COUNT; ++i) {
            samples[i] = 0;
        }
        samples_idx = 0;
        avg_ops = 0;
    }
    std::atomic<int64_t> last_sample_time = ClockTime::intervalMs();
    std::atomic<int64_t> last_sample_count = 0;
    std::atomic<int64_t> samples[ATOMIC_STATS_SAMPLES_COUNT] = {0};
    std::atomic<int32_t> samples_idx = 0;
    std::atomic<int64_t> avg_ops = 0;
};

template <typename TYPE, typename STATISTICS, int stats_count>
class AtomicStatisticsHelper final : private Noncopyable {
public:
    static inline void clear(int start_idx, int end_index) {
        stat_.clear(start_idx, end_index);
        for (int i = 0; i < stats_count; ++i) {
            metric_[i].clear();
        }
    }

    static inline void subStat(TYPE type, int64_t count) {
        stat_.subStat(type, count);
    }

    static inline void addStat(TYPE type, int64_t count) {
        stat_.addStat(type, count);
    }

    static inline int64_t getStat(TYPE type) {
        return stat_.getStat(type);
    }

    static inline int64_t getStatOps(TYPE type) {
        return metric_[type].avg_ops;
    }

    static void calcStatistics(int start = 0, int end = stats_count - 1) {
        for (int i = start; i <= end; ++i) {
            trackMetric(i, stat_.getStat((TYPE)i));
        }
    }

private:
    static void trackMetric(int type, int64_t curr_count) {
        int64_t interval_time = ClockTime::intervalMs();
        int64_t interval = interval_time - metric_[type].last_sample_time;
        int64_t ops = curr_count - metric_[type].last_sample_count;

        int64_t ops_sec = interval > 0 ? (ops * 1000 / interval) : 0;

        int32_t idx = metric_[type].samples_idx;
        metric_[type].samples[idx] = ops_sec;
        metric_[type].samples_idx = (idx + 1) % ATOMIC_STATS_SAMPLES_COUNT;

        metric_[type].last_sample_time = interval_time;
        metric_[type].last_sample_count = curr_count;

        int64_t sum = 0;
        for (auto &count : metric_[type].samples) {
            sum += count.load();
        }
        metric_[type].avg_ops = sum / ATOMIC_STATS_SAMPLES_COUNT;
    }

private:
    static STATISTICS stat_;
    static AtomicMetric metric_[stats_count];
};

template <typename TYPE, typename STATISTICS, int stats_count>
STATISTICS AtomicStatisticsHelper<TYPE, STATISTICS, stats_count>::stat_;

template <typename TYPE, typename STATISTICS, int stats_count>
AtomicMetric AtomicStatisticsHelper<TYPE, STATISTICS, stats_count>::metric_[stats_count];

} // namespace tair::common
