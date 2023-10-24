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

#include <deque>

#include "common/Assert.hpp"
#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"

namespace tair::common {

template <typename T>
class BlockingQueue final : private Noncopyable {
public:
    BlockingQueue() = default;
    ~BlockingQueue() = default;

    template <typename TT>
    size_t put(TT &&t) {
        LockGuard lock(mutex_);
        queue_.emplace_back(std::forward<TT>(t));
        not_empty_cond_.notifyOne();
        return queue_.size();
    }

    template <typename TT>
    size_t put_front(TT &&t) {
        LockGuard lock(mutex_);
        queue_.emplace_front(std::forward<TT>(t));
        not_empty_cond_.notifyOne();
        return queue_.size();
    }

    T take() {
        UniqueLock lock(mutex_);
        while (queue_.empty()) {
            not_empty_cond_.wait(lock);
        }
        runtimeAssert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        return front;
    }

    bool tryTake(T &t) {
        LockGuard lock(mutex_);
        if (!queue_.empty()) {
            t = std::move(queue_.front());
            queue_.pop_front();
            return true;
        }
        return false;
    }

    template <class _Rep, class _Period>
    bool take(T &t, const std::chrono::duration<_Rep, _Period> &timeout) {
        UniqueLock lock(mutex_);
        if (queue_.empty()) {
            not_empty_cond_.waitFor(lock, timeout);
        }
        if (!queue_.empty()) {
            t = std::move(queue_.front());
            queue_.pop_front();
            return true;
        }
        return false;
    }

    size_t size() const {
        LockGuard lock(mutex_);
        return queue_.size();
    }

    bool empty() const {
        LockGuard lock(mutex_);
        return queue_.empty();
    }

    void clear() {
        LockGuard lock(mutex_);
        queue_.clear();
    }

private:
    mutable Mutex mutex_;
    Condition not_empty_cond_ GUARDED_BY(mutex_);
    std::deque<T> queue_ GUARDED_BY(mutex_);
};

} // namespace tair::common
