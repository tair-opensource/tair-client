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

#include "common/ThreadPool.hpp"

using tair::common::ThreadPool;

TEST(THREAD_POOL, ONLY_TEST) {
    std::atomic<int> count = 0;
    ASSERT_EQ(0, count);

    ThreadPool pool(3, "pool");
    pool.start();
    pool.setMaxQueueSize(1);
    for (int i = 0; i <= 100; ++i) {
        pool.putTask([i, &count]() {
            count += i;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(0U, pool.queueSize());

    ASSERT_EQ("pool", pool.getName());
    ASSERT_EQ(5050, count);

    ThreadPool pool2(1, "pool2");
    bool not_start_called = false;
    pool2.putTask([&]() { not_start_called = true; });
    ASSERT_TRUE(not_start_called);
    pool2.start();
    pool2.putTask([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    pool2.putTask([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    pool2.putTask([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_LT(0U, pool2.queueSize());
    pool2.stop();
    ASSERT_EQ(0U, pool2.queueSize());
}
