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
#include <thread>

#include "common/CountDownLatch.hpp"
#include "common/ThreadLocalSingleton.hpp"

using tair::common::ThreadLocalSingleton;
using tair::common::CountDownLatch;

TEST(THREAD_LOCAL_SINGLETON_TEST, ONLY_TEST) {
    static std::atomic<int> construction_count = 0;
    static std::atomic<int> deconstruction_count = 0;

    class Test {
    public:
        Test() {
            construction_count++;
        }
        ~Test() {
            deconstruction_count++;
        }
    };
    int thread_count = 5;
    {
        CountDownLatch latch(thread_count);
        for (int i = 0; i < thread_count; ++i) {
            std::thread thread([&]() {
                ThreadLocalSingleton<Test>::instance();
                latch.countDown();
            });
            thread.detach();
        }
        latch.wait();
        while (deconstruction_count != thread_count) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        size_t all_size = 0;
        ThreadLocalSingleton<Test>::rangeForAllThreadData([&all_size](const auto &datas) {
            all_size = datas.size();
        });
        ASSERT_EQ(0U, all_size);
    }
    ASSERT_EQ(thread_count, construction_count);
    ASSERT_EQ(thread_count, deconstruction_count);
}
