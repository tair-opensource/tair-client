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
#include "network/EventLoopThread.hpp"
#include "gtest/gtest.h"

using tair::network::Duration;
using tair::network::TimerId;
using tair::network::EventLoop;
using tair::network::EventLoopThread;

TEST(EVENT_LOOP_TEST, TIMER_THREAD_TEST) {
    EventLoopThread loop_thread("TimerLoop");
    loop_thread.start();
    auto loop = loop_thread.loop();

    ASSERT_FALSE(loop->isInLoopThread());
    ASSERT_NE(std::this_thread::get_id(), loop_thread.tid());

    std::atomic<int> count = 0;
    TimerId everyTimerId = loop->runEveryTimer(Duration(180 * Duration::kMillisecond), [&](EventLoop *) {
        ASSERT_EQ(std::this_thread::get_id(), loop_thread.tid());
        count++;
    });
    loop->runAfterTimer(Duration(1 * Duration::kSecond), [&](EventLoop *loop) {
        ASSERT_EQ(5, count);
        count = -1;
        loop->cancelTimer(everyTimerId);
        loop_thread.stop();
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    ASSERT_EQ("TimerLoop", loop_thread.name());
    ASSERT_EQ(-1, count);
}
