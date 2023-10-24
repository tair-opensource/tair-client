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
#include <atomic>

#include "common/CountDownLatch.hpp"
#include "network/EventLoopThreadPool.hpp"

#include "gtest/gtest.h"

using tair::common::CountDownLatch;
using tair::network::Duration;
using tair::network::TimerId;
using tair::network::EventLoop;
using tair::network::EventLoopThreadPool;

TEST(EVENT_LOOP_TEST, TIMER_THREAD_POOL_NO_POOL_TEST) {
    EventLoop base_loop("base_loop");
    EventLoopThreadPool pool(&base_loop, 0, "timerLoopPool");

    pool.start();

    ASSERT_EQ(0, pool.ioThreadNum());
    ASSERT_TRUE(pool.isRunning());

    CountDownLatch latch(2);

    pool.runInNextlLoop([&](EventLoop *) {
        ASSERT_EQ("base_loop", base_loop.getName());
        latch.countDown();
    });

    std::srand(std::time(nullptr));
    pool.runInLoopByHash(std::rand(), [&](EventLoop *) {
        ASSERT_EQ("base_loop", base_loop.getName());
        latch.countDown();
    });

    latch.wait();
    pool.stop();
}
