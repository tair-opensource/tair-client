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

#include "common/CoroutineThread.hpp"

using tair::common::CoroutineThread;

void coroutine_thread_test(bool use_shared_stack) {
    std::atomic<int> count = 0;
    ASSERT_EQ(0, count);

    CoroutineThread co_thread(use_shared_stack, "test");
    co_thread.start();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto task = [&count](int id, int n) {
        for (int i = 0; i < n; ++i) {
            count++;
            // fprintf(stderr, "task id: %d, seq: %d\n", id, i + 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            CoroutineThread::yield();
        }
    };

    ASSERT_TRUE(co_thread.putTask([&]() {
        task(1, 10);
    }));
    ASSERT_TRUE(co_thread.putTask([&]() {
        task(2, 20);
    }));
    ASSERT_TRUE(co_thread.putTask([&]() {
        task(3, 30);
    }));

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ASSERT_EQ(0U, co_thread.queueSize());
    ASSERT_EQ(3U, co_thread.coroutineSize());

    co_thread.stop();

    ASSERT_EQ(0U, co_thread.queueSize());
    ASSERT_EQ(0U, co_thread.coroutineSize());

    ASSERT_EQ(60, count);
}

TEST(COROUTINE_THREAD, ONLY_TEST) {
    coroutine_thread_test(false);
#ifdef NDBUG
    // Who knowns how to add sanitizer support in debug mode？
    coroutine_thread_test(true);
#endif
}
