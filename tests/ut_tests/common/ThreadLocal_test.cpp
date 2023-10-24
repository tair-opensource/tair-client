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
#include "common/ThreadLocal.hpp"

using tair::common::ThreadLocal;
using tair::common::CountDownLatch;

TEST(THREAD_LOCAL_TEST, ONLY_TEST) {
    static std::atomic<int> construction_count = 0;
    static std::atomic<int> destruction_count = 0;

    class Test {
    public:
        Test() {
            construction_count++;
        }
        ~Test() {
            destruction_count++;
        }
    };

    {
        ThreadLocal<Test> test;
        ASSERT_EQ(0U, test.getAllPointers().size());
        ASSERT_EQ(0, construction_count);
        ASSERT_EQ(0, destruction_count);

        CountDownLatch latch(5);
        for (int i = 0; i < 5; ++i) {
            std::thread thread([&]() {
                test.pointer();
                std::this_thread::sleep_for(std::chrono::seconds(1));
                latch.countDown();
            });
            thread.detach();
        }
        latch.wait();
        ASSERT_EQ(0, latch.getCount());
        ASSERT_EQ(5U, test.getAllPointers().size());
    }

    ASSERT_EQ(5, construction_count);
    ASSERT_EQ(5, destruction_count);
}
