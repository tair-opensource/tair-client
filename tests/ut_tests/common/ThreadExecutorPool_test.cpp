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

#include "common/ThreadExecutorPool.hpp"

using tair::common::ThreadExecutorPool;

TEST(THREAD_EXECUTOR_POOL, ONLY_TEST) {
    std::atomic<int> count = 0;
    ASSERT_EQ(0, count);

    ThreadExecutorPool pool(3, 100, "test");
    pool.start();
    for (int i = 0; i <= 100; ++i) {
        pool.getNextExecutor()->putTask([i, &count]() {
            count += i;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(0U, pool.allExecutorQueueSize());

    ASSERT_EQ("th-test", pool.getName());
    ASSERT_EQ(5050, count);

    ThreadExecutorPool pool2(1, 100, "test2");
    pool2.start();
    pool2.getNextExecutor()->putTask([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    pool2.getNextExecutor()->putTask([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    ASSERT_EQ(1U, pool2.allExecutorQueueSize());
    pool2.stop();
    ASSERT_EQ(0U, pool2.allExecutorQueueSize());
}
