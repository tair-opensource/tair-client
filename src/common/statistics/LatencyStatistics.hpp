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

#include <array>
#include <cstddef>
#include <cstdint>
#include <ostream>

#include "common/ClockTime.hpp"
#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"

#include "fmt/format.h"

namespace tair::common {

class LatencyMetric : private Noncopyable {
public:
    LatencyMetric() = default;
    ~LatencyMetric() = default;

    void addSample(size_t latency_us) {
        latency_us_sum_ += latency_us;
        if (latency_us_max_ < latency_us) {
            latency_us_max_ = latency_us;
        }
        if (latency_us_min_ > latency_us) {
            latency_us_min_ = latency_us;
        }
        size_t bucket = calcBucket(latency_us);
        latency_buckets_[bucket]++;
        op_count_++;
    }

    void clear() {
        op_count_ = 0;
        latency_us_sum_ = 0;
        latency_us_max_ = 0;
        ::memset(latency_buckets_, 0, sizeof(latency_buckets_));
    }

    size_t getOpCount() const { return op_count_; }
    size_t getLatencyUsSum() const { return latency_us_sum_; }
    size_t getLatencyUsMin() const { return op_count_ == 0 ? op_count_ : latency_us_min_; }
    size_t getLatencyUsMax() const { return latency_us_max_; }
    size_t getLatencyUsAvg() const { return op_count_ ? latency_us_sum_ / op_count_ : 0; }

    double getLatencyUsPerc(double perc) const {
        size_t calc_count = op_count_ * (perc / 100) + 0.5;
        if (calc_count >= op_count_) {
            return latency_us_max_;
        }
        size_t last_bucket_count = 0;
        size_t i = 0;
        while (i <= LATENCY_ORDER_MAX && calc_count > 0) {
            last_bucket_count = latency_buckets_[i];
            if (calc_count > last_bucket_count) {
                calc_count -= last_bucket_count;
                i++;
            } else {
                break;
            }
        }
        size_t latency_start = LATENCY_BUCKETS_START[i];
        double latency_perc = latency_start;
        if (calc_count && last_bucket_count) {
            size_t latency_diff = latency_start;
            latency_perc += latency_diff * ((double)calc_count / last_bucket_count);
        }
        return latency_perc < latency_us_max_ ? latency_perc : latency_us_max_;
    }

    LatencyMetric &operator+=(const LatencyMetric &rhs) {
        op_count_ += rhs.op_count_;
        latency_us_sum_ += rhs.latency_us_sum_;
        if (latency_us_max_ < rhs.latency_us_max_) {
            latency_us_max_ = rhs.latency_us_max_;
        }
        for (size_t i = 0; i <= LATENCY_ORDER_MAX; ++i) {
            latency_buckets_[i] += rhs.latency_buckets_[i];
        }
        return *this;
    }

    std::string toString() const {
        std::string out;
        size_t latency_avg = op_count_ > 0 ? latency_us_sum_ / op_count_ : 0;
        fmt::format_to(std::back_inserter(out), "{}|{}|{}", op_count_, latency_avg, latency_us_max_);
        for (size_t i = 0; i <= LATENCY_ORDER_MAX; ++i) {
            fmt::format_to(std::back_inserter(out), "|{}", latency_buckets_[i]);
        }
        return out;
    }

    std::string toPercString(const std::vector<double> &percs) const {
        std::string out;
        for (auto &perc : percs) {
            fmt::format_to(std::back_inserter(out), "p{}={:.3f},", perc, getLatencyUsPerc(perc));
        }
        if (!out.empty()) {
            out.erase(out.end() - 1);
        }
        return out;
    }

    static const char *getFormattedMetricTitle() {
        return "  count|   avg(us)|   max(us)|"
               "  <1us|  <2us|  <4us|  <8us| <16us| <32us| <64us|<128us|<256us|<512us|"
               "  <1ms|  <2ms|  <4ms|  <8ms| <16ms| <32ms| <65ms|<131ms|<262ms|<524ms|"
               "   <1s|   <2s|   <4s|   <8s|  <16s|  <33s|  >33s";
    }

    std::string getFormattedMetric() const {
        std::string out;
        size_t latency_avg = getLatencyUsAvg();
        fmt::format_to(std::back_inserter(out), "{:>7}|{:>10}|{:>10}", op_count_, latency_avg, latency_us_max_);
        for (size_t i = 0; i <= LATENCY_ORDER_MAX; ++i) {
            fmt::format_to(std::back_inserter(out), "|{:>6}", latency_buckets_[i]);
        }
        return out;
    }

public:
    /*
     * Bucket     Latency
     * 0          < 1us
     * 1          < 2us
     * 2          < 4us
     * 3          < 8us
     *      ...
     * 16         < 65536us (~65ms)
     *      ...
     * 20         < 1048576us (~1s)
     *      ...
     * 25         < 33554432us (~33s)
     * 26         >= 33554432us (33s)
     */
    constexpr static size_t LATENCY_ORDER_MAX = 26;

    constexpr static size_t calcBucket(size_t x) {
        size_t bucket = x > 0 ? 64 - __builtin_clzll(x) : 0;
        if (bucket > LATENCY_ORDER_MAX) {
            bucket = LATENCY_ORDER_MAX;
        }
        return bucket;
    }

    // clang-format off
    constexpr const static size_t LATENCY_BUCKETS_START[LATENCY_ORDER_MAX + 1] = {
        0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
        65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432,
    };
    // clang-format on

private:
    size_t op_count_ = 0;
    size_t latency_us_sum_ = 0;
    size_t latency_us_min_ = std::numeric_limits<size_t>::max();
    size_t latency_us_max_ = 0;
    size_t latency_buckets_[LATENCY_ORDER_MAX + 1] = {0};
};

template <int LATENCY_STAT_COUNT>
class LatencyStatistics : private Noncopyable {
public:
    LatencyStatistics() {
        latency_entrys_ = std::make_unique<LatencyMetric[]>(LATENCY_STAT_COUNT);
    }

    virtual ~LatencyStatistics() = default;

    void addSample(size_t idx, size_t latency_us) {
        LockGuard lock(mutex_);
        latency_entrys_[idx].addSample(latency_us);
    }

    size_t getLatencyMax(size_t idx) {
        LockGuard lock(mutex_);
        return latency_entrys_[idx].getLatencyUsMax();
    }

    size_t getLatencySum(size_t idx) {
        LockGuard lock(mutex_);
        return latency_entrys_[idx].getLatencyUsSum();
    }

    size_t getLatencyCount(size_t idx) {
        LockGuard lock(mutex_);
        return latency_entrys_[idx].getOpCount();
    }

    size_t getLatencyAvg(size_t idx) {
        LockGuard lock(mutex_);
        return latency_entrys_[idx].getLatencyUsAvg();
    }

    double getLatencyPerc(size_t idx, double perc) {
        LockGuard lock(mutex_);
        return latency_entrys_[idx].getLatencyUsPerc(perc);
    }

    std::unique_ptr<LatencyMetric[]> getMetricsAndReinit() {
        LockGuard lock(mutex_);
        auto entrys = std::make_unique<LatencyMetric[]>(LATENCY_STAT_COUNT);
        entrys.swap(latency_entrys_);
        return entrys;
    }

    void mergeMetrics(const std::unique_ptr<LatencyMetric[]> &metrics, int start_idx = 0, int end_index = INT32_MAX) {
        LockGuard lock(mutex_);
        for (size_t i = 0; i < LATENCY_STAT_COUNT; ++i) {
            latency_entrys_[i] += metrics[i];
        }
    }

    void clear(int start_idx = 0, int end_index = INT32_MAX) {
        LockGuard lock(mutex_);
        for (int i = start_idx; i < LATENCY_STAT_COUNT && i <= end_index; ++i) {
            latency_entrys_[i].clear();
        }
    }

protected:
    mutable Mutex mutex_;
    std::unique_ptr<LatencyMetric[]> latency_entrys_ GUARDED_BY(mutex_);
};

} // namespace tair::common
