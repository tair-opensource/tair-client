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

#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"

namespace tair::common {

class CountDownLatch : private Noncopyable {
public:
    CountDownLatch() = default;
    explicit CountDownLatch(int count)
        : count_(count) {}
    ~CountDownLatch() = default;

    void wait() {
        UniqueLock lock(mutex_);
        while (count_ > 0) {
            condition_.wait(lock);
        }
    }

    template <class _Rep, class _Period>
    bool waitFor(const std::chrono::duration<_Rep, _Period> &timeout) {
        UniqueLock lock(mutex_);
        if (count_ > 0) {
            condition_.waitFor(lock, timeout);
        }
        return (count_ == 0);
    }

    void countDown() {
        LockGuard lock(mutex_);
        count_--;
        if (count_ == 0) {
            condition_.notifyAll();
        }
    }

    int getCount() const {
        LockGuard lock(mutex_);
        return count_;
    }

private:
    mutable Mutex mutex_;
    Condition condition_ GUARDED_BY(mutex_);
    int count_ GUARDED_BY(mutex_) = 1;
};

} // namespace tair::common
