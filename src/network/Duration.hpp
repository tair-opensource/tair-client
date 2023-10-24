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

#include <sys/time.h>

#include <cstdint>

namespace tair::network {

// A Duration represents the elapsed time between two instants
// as an int64 nanosecond count. The representation limits the
// largest representable duration to approximately 290 years.
class Duration {
public:
    static const int64_t kNanosecond;  // = 1LL
    static const int64_t kMicrosecond; // = 1000
    static const int64_t kMillisecond; // = 1000 * kMicrosecond
    static const int64_t kSecond;      // = 1000 * kMillisecond
    static const int64_t kMinute;      // = 60 * kSecond
    static const int64_t kHour;        // = 60 * kMinute

public:
    Duration() = default;
    explicit Duration(const struct timeval &t)
        : nanoseconds_(t.tv_sec * kSecond + t.tv_usec * kMicrosecond) {}
    explicit Duration(int nanoseconds)
        : nanoseconds_(nanoseconds) {}
    explicit Duration(int64_t nanoseconds)
        : nanoseconds_(nanoseconds) {}
    explicit Duration(double seconds)
        : nanoseconds_(int64_t(seconds * kSecond)) {}

    // Nanoseconds returns the duration as an integer nanosecond count.
    inline int64_t nanoseconds() const {
        return nanoseconds_;
    }

    // These methods return double because the dominant
    // use case is for printing a floating point number like 1.5s, and
    // a truncation to integer would make them not useful in those cases.

    // Seconds returns the duration as a floating point number of seconds.
    inline double seconds() const {
        return double(nanoseconds_) / kSecond;
    }

    inline double milliseconds() const {
        return double(nanoseconds_) / kMillisecond;
    }

    inline double microseconds() const {
        return double(nanoseconds_) / kMicrosecond;
    }

    inline double minutes() const {
        return double(nanoseconds_) / kMinute;
    }

    inline double hours() const {
        return double(nanoseconds_) / kHour;
    }

    inline void to(struct timeval *t) const {
        t->tv_sec = (long)(nanoseconds_ / kSecond);
        t->tv_usec = (long)(nanoseconds_ % kSecond) / (long)kMicrosecond;
    }

    inline struct timeval timeVal() const {
        struct timeval t;
        to(&t);
        return t;
    }

    inline bool isZero() const {
        return nanoseconds_ == 0;
    }

    inline bool operator<(const Duration &rhs) const {
        return nanoseconds_ < rhs.nanoseconds_;
    }

    inline bool operator<=(const Duration &rhs) const {
        return nanoseconds_ <= rhs.nanoseconds_;
    }

    inline bool operator>(const Duration &rhs) const {
        return nanoseconds_ > rhs.nanoseconds_;
    }

    inline bool operator>=(const Duration &rhs) const {
        return nanoseconds_ >= rhs.nanoseconds_;
    }

    inline bool operator==(const Duration &rhs) const {
        return nanoseconds_ == rhs.nanoseconds_;
    }

    inline Duration operator+=(const Duration &rhs) {
        nanoseconds_ += rhs.nanoseconds_;
        return *this;
    }

    inline Duration operator-=(const Duration &rhs) {
        nanoseconds_ -= rhs.nanoseconds_;
        return *this;
    }

    inline Duration operator*=(int nanoseconds) {
        nanoseconds_ *= nanoseconds;
        return *this;
    }

    inline Duration operator/=(int nanoseconds) {
        nanoseconds_ /= nanoseconds;
        return *this;
    }

private:
    int64_t nanoseconds_ = 0; // nanoseconds
};

} // namespace tair::network
