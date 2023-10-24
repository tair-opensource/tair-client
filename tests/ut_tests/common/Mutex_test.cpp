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

#include <atomic>

#include "common/ClockTime.hpp"
#include "common/Mutex.hpp"

using tair::common::Mutex;
using tair::common::ReadWriteLock;
using tair::common::ReadLockGuard;
using tair::common::WriteLockGuard;
using tair::common::ClockTime;

TEST(MUTEX_AVG_TIME_TEST, ONLY_TEST)
NO_THREAD_SAFETY_ANALYSIS {
    constexpr int test_count = 10000;
    constexpr int recursive_count = 10;
    int64_t start_time = 0;
    double used_time = 0, avg_ns = 0;

    Mutex mutex;
    start_time = ClockTime::nowNs();
    for (int i = 0; i < test_count; ++i) {
        mutex.lock();
        mutex.unlock();
    }
    used_time = ClockTime::nowNs() - start_time;
    avg_ns = used_time / test_count;
    std::cerr << "mutex lock and unlock avg used: " << avg_ns << " ns" << std::endl;

    std::recursive_mutex recursive_mutex;
    start_time = ClockTime::nowNs();
    for (int i = 0; i < test_count; ++i) {
        for (int j = 0; j < recursive_count; ++j) {
            recursive_mutex.lock();
        }
        for (int j = 0; j < recursive_count; ++j) {
            recursive_mutex.unlock();
        }
    }
    used_time = ClockTime::nowNs() - start_time;
    avg_ns = used_time / test_count / recursive_count;
    std::cerr << "recursive_mutex lock and unlock: " << avg_ns << " ns" << std::endl;

    ReadWriteLock rw_lock;
    start_time = ClockTime::nowNs();
    for (int i = 0; i < test_count; ++i) {
        rw_lock.readLock();
        rw_lock.unlock();
    }
    used_time = ClockTime::nowNs() - start_time;
    avg_ns = used_time / test_count;
    std::cerr << "ReadWriteLock rlock and unlock avg used: " << avg_ns << " ns" << std::endl;

    start_time = ClockTime::nowNs();
    for (int i = 0; i < test_count; ++i) {
        for (int j = 0; j < recursive_count; ++j) {
            rw_lock.readLock();
        }
        for (int j = 0; j < recursive_count; ++j) {
            rw_lock.unlock();
        }
    }
    used_time = ClockTime::nowNs() - start_time;
    avg_ns = used_time / test_count / recursive_count;
    std::cerr << "ReadWriteLock recursive rlock and unlock avg uesd: " << avg_ns << " ns" << std::endl;

    start_time = ClockTime::nowNs();
    for (int i = 0; i < test_count; ++i) {
        rw_lock.writeLock();
        rw_lock.unlock();
    }
    used_time = ClockTime::nowNs() - start_time;
    avg_ns = used_time / test_count;
    std::cerr << "ReadWriteLock wlock and unlock avg used: " << avg_ns << " ns" << std::endl;

    std::atomic<int> count;
    start_time = ClockTime::nowNs();
    for (int i = 0; i < test_count; ++i) {
        count++;
    }
    used_time = ClockTime::nowNs() - start_time;
    avg_ns = used_time / test_count;
    std::cerr << "std::atomic<int> incr avg used: " << avg_ns << " ns" << std::endl;
}
