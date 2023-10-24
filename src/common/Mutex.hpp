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

#include <condition_variable>
#include <mutex>
#include <set>

#include "common/Assert.hpp"

// Thread safety annotations {
// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x) // no-op
#endif

#define CAPABILITY(x) THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define RELEASE_GENERIC(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(release_generic_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
    THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

namespace tair::common {

// NOTE: Wrappers for std::mutex and std::unique_lock are provided so that
// we can annotate them with thread safety attributes and use the
// -Wthread-safety warning with clang. The standard library types cannot be
// used directly because they do not provide the required annotations.
class CAPABILITY("mutex") Mutex final {
public:
    Mutex() = default;
    ~Mutex() = default;

    bool trylock() TRY_ACQUIRE(true) { return mut_.try_lock(); }
    void lock() ACQUIRE() { mut_.lock(); }
    void unlock() RELEASE() { mut_.unlock(); }
    void assertHeld() ASSERT_CAPABILITY() {}

private:
    friend class UniqueLock;
    friend class LockGuard;
    std::mutex &native_handle() { return mut_; }

private:
    std::mutex mut_;
};

class SCOPED_CAPABILITY UniqueLock final {
    typedef std::unique_lock<std::mutex> UniqueLockImp;

public:
    explicit UniqueLock(Mutex &m) ACQUIRE(m)
        : ul_(m.native_handle()) {}
    ~UniqueLock() RELEASE() = default;
    UniqueLockImp &native_handle() { return ul_; }

private:
    UniqueLockImp ul_;
};

class SCOPED_CAPABILITY LockGuard final {
    typedef std::lock_guard<std::mutex> LockGuardImp;

public:
    explicit LockGuard(Mutex &m) ACQUIRE(m)
        : lg_(m.native_handle()) {}
    ~LockGuard() RELEASE() = default;

private:
    LockGuardImp lg_;
};

class MultiLockGuard final {
public:
    explicit MultiLockGuard(const std::set<Mutex *> &mutexs)
        : mutexs_(mutexs) {
        for (auto &mutex : mutexs_) {
            mutex->lock();
        }
    }
    ~MultiLockGuard() {
        for (auto &mutex : mutexs_) {
            mutex->unlock();
        }
    }

private:
    const std::set<Mutex *> &mutexs_;
};

class Condition final {
public:
    Condition() = default;
    ~Condition() = default;

    void wait(UniqueLock &lock) {
        cond_.wait(lock.native_handle());
    }

    template <class _Rep, class _Period>
    void waitFor(UniqueLock &lock, const std::chrono::duration<_Rep, _Period> &timeout) {
        cond_.wait_for(lock.native_handle(), timeout);
    }

    void notifyOne() {
        cond_.notify_one();
    }

    void notifyAll() {
        cond_.notify_all();
    }

private:
    std::condition_variable cond_;
};

class CAPABILITY("mutex") ReadWriteLock final {
public:
    ReadWriteLock() {
        runtimeAssert(pthread_rwlockattr_init(&rw_attr_) == 0);
#ifdef __linux__
        pthread_rwlockattr_setkind_np(&rw_attr_, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
#endif
        runtimeAssert(pthread_rwlock_init(&rw_lock_, &rw_attr_) == 0);
    }

    ~ReadWriteLock() {
        runtimeAssert(pthread_rwlock_destroy(&rw_lock_) == 0);
        runtimeAssert(pthread_rwlockattr_destroy(&rw_attr_) == 0);
    }

    void readLock() ACQUIRE_SHARED() {
        runtimeAssert(pthread_rwlock_rdlock(&rw_lock_) == 0);
    }

    void writeLock() ACQUIRE() {
        runtimeAssert(pthread_rwlock_wrlock(&rw_lock_) == 0);
    }

    void assertHeld() ASSERT_CAPABILITY() {
    }
    void assertSharedHeld() ASSERT_SHARED_CAPABILITY() {
    }
    void assertReadHeld() ASSERT_SHARED_CAPABILITY() {
    }

    bool tryReadLock() TRY_ACQUIRE_SHARED(true) {
        return pthread_rwlock_tryrdlock(&rw_lock_) == 0;
    }

    bool tryWriteLock() TRY_ACQUIRE(true) {
        return pthread_rwlock_trywrlock(&rw_lock_) == 0;
    }

    void sharedLock() ACQUIRE_SHARED() {
        readLock();
    }

    void exclusiveLock() ACQUIRE() {
        writeLock();
    }

    bool trySharedLock() TRY_ACQUIRE_SHARED(true) {
        return tryReadLock();
    }

    bool tryExclusiveLock() TRY_ACQUIRE(true) {
        return tryWriteLock();
    }

    void unlock() RELEASE_GENERIC() {
        runtimeAssert(pthread_rwlock_unlock(&rw_lock_) == 0);
    }

private:
    pthread_rwlock_t rw_lock_;
    pthread_rwlockattr_t rw_attr_;
};

using SharedExclusiveLock = ReadWriteLock;

class SCOPED_CAPABILITY ReadLockGuard final {
public:
    explicit ReadLockGuard(ReadWriteLock &lock) ACQUIRE_SHARED(lock)
        : lock_(lock) {
        lock_.readLock();
    }
    ~ReadLockGuard() RELEASE() {
        lock_.unlock();
    }

private:
    ReadWriteLock &lock_;
};

using SharedLockGuard = ReadLockGuard;

class SCOPED_CAPABILITY WriteLockGuard final {
public:
    explicit WriteLockGuard(ReadWriteLock &lock) ACQUIRE(lock)
        : lock_(lock) {
        lock_.writeLock();
    }
    ~WriteLockGuard() RELEASE() {
        lock_.unlock();
    }

private:
    ReadWriteLock &lock_;
};

using ExclusiveLockGuard = WriteLockGuard;

} // namespace tair::common
