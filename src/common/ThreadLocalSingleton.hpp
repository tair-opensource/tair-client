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

#include <functional>
#include <pthread.h>
#include <unordered_set>

#include "common/Assert.hpp"
#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"

namespace tair::common {

template <typename T>
class ThreadLocalSingleton : private Noncopyable {
public:
    ThreadLocalSingleton() = delete;
    ~ThreadLocalSingleton() = delete;

    static inline T *instance() {
        if (!t_value_) {
            t_value_ = new T();
            deleter_.set(t_value_);
            LockGuard lock(mutex_);
            runtimeAssert(values_.find(t_value_) == values_.end());
            values_.insert(t_value_);
        }
        return t_value_;
    }

    using Callback = std::function<void(const std::unordered_set<T *> &)>;

    static void rangeForAllThreadData(const Callback &callback) {
        LockGuard lock(mutex_);
        callback(values_);
    }

private:
    static void destructor(void *obj) {
        {
            LockGuard lock(mutex_);
            values_.erase((T *)obj);
        }
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy;
        (void)dummy;
        delete (T *)obj;
        t_value_ = nullptr;
    }

    class Deleter {
    public:
        Deleter() {
            ::pthread_key_create(&pkey_, &ThreadLocalSingleton::destructor);
        }

        ~Deleter() {
            ::pthread_key_delete(pkey_);
        }

        void set(T *obj) {
            runtimeAssert(pthread_getspecific(pkey_) == nullptr);
            ::pthread_setspecific(pkey_, obj);
        }

    private:
        pthread_key_t pkey_;
    };

private:
    static Mutex mutex_;
    static std::unordered_set<T *> values_ GUARDED_BY(mutex_);
    static thread_local T *t_value_;

    // We cannot call destructor for thread local
    static Deleter deleter_;
};

template <typename T>
Mutex ThreadLocalSingleton<T>::mutex_;

template <typename T>
std::unordered_set<T *> ThreadLocalSingleton<T>::values_;

template <typename T>
thread_local T *ThreadLocalSingleton<T>::t_value_ = nullptr;

template <typename T>
typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::deleter_;

} // namespace tair::common
