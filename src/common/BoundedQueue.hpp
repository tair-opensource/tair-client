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

#include <queue>

#include "common/Assert.hpp"
#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"

namespace tair::common {

template <typename T>
class BoundedQueue final : private Noncopyable {
public:
    BoundedQueue(size_t max_size)
        : max_size_(max_size) {}
    ~BoundedQueue() = default;

    template <typename TT>
    bool put(TT &&t, bool force) {
        LockGuard lock(mutex_);
        if (queue_.size() > max_size_ && !force) {
            return false;
        }
        queue_.push(std::forward<TT>(t));
        return true;
    }

    bool take(T &t) {
        LockGuard lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        t = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void clear() {
        std::queue<T> empty;
        {
            LockGuard lock(mutex_);
            return queue_.swap(empty);
        }
    }

    size_t size() const {
        LockGuard lock(mutex_);
        return queue_.size();
    }

    bool empty() const {
        LockGuard lock(mutex_);
        return queue_.empty();
    }

private:
    const size_t max_size_;
    mutable Mutex mutex_;
    std::queue<T> queue_ GUARDED_BY(mutex_);
};

} // namespace tair::common
