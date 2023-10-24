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
#if defined(__linux__) && defined(USE_SANITIZER)

#include "common/Compiler.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h> // for dlsym()
#include <mutex>
#include <new>
#include <pthread.h>

static pthread_rwlock_t rw_lock_;
static pthread_rwlockattr_t rw_attr_;
static bool rw_inited = false;

#define hook_assert(e) (__builtin_expect(!(e), 0) ? abort() : (void)0)

static void rw_lock_init() {
    memset(&rw_lock_, 0, sizeof(rw_lock_));
    memset(&rw_attr_, 0, sizeof(rw_attr_));
    hook_assert(pthread_rwlockattr_init(&rw_attr_) == 0);
    hook_assert(pthread_rwlockattr_setkind_np(&rw_attr_, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP) == 0);
    hook_assert(pthread_rwlock_init(&rw_lock_, &rw_attr_) == 0);
    rw_inited = true;
}

static void memory_hook_exclusive_lock() {
    if (unlikely(!rw_inited)) {
        rw_lock_init();
    }
    hook_assert(pthread_rwlock_wrlock(&rw_lock_) == 0);
}

static void memory_hook_shared_lock() {
    if (unlikely(!rw_inited)) {
        rw_lock_init();
    }
    hook_assert(pthread_rwlock_rdlock(&rw_lock_) == 0);
}

static void memory_hook_unlock() {
    hook_assert(pthread_rwlock_unlock(&rw_lock_) == 0);
}

static void hook_prefork() {
    memory_hook_exclusive_lock();
}

static void hook_postfork_parent() {
    memory_hook_unlock();
}

static void hook_postfork_child() {
    rw_lock_init();
}

bool hook_inited = false;

static const size_t static_buffer_size = 10485760;
static char static_buffer[static_buffer_size];
static char *pos = static_buffer;

void initMemoryHookForWorkaroundASAN() {
    hook_assert(pthread_atfork(hook_prefork, hook_postfork_parent, hook_postfork_child) == 0);
    hook_inited = true;
}

static bool dlsymUsed() {
    return !hook_inited;
}

static bool isStaticAddr(void *ptr) {
    return ptr >= static_buffer && ptr < static_buffer + static_buffer_size;
}

#define alignment16(len) ((len + 0x0F) & (~0x0F))

static void *staticAlloc(size_t size) {
    size_t alignment_size = alignment16(size);
    void *ptr = pos;
    pos += alignment_size;
    hook_assert(pos < static_buffer + static_buffer_size);
    return ptr;
}

// ----------------------------------------------------------------------------------------------

// If an application creates a thread before doing any allocation in the main thread,
// then calls fork(2) in the main thread followed by ASAN check in the child process,
// a race can occur that results in deadlock within the child.

__BEGIN_DECLS

void *malloc(size_t size) {
    if (unlikely(dlsymUsed())) {
        return staticAlloc(size);
    }
    static void *(*libc_malloc)(size_t size) = nullptr;
    if (unlikely(!libc_malloc)) {
        libc_malloc = (decltype(libc_malloc))dlsym(RTLD_NEXT, "malloc");
    }
    memory_hook_shared_lock();
    void *ptr = libc_malloc(size);
    memory_hook_unlock();
    return ptr;
}

void *calloc(size_t nmemb, size_t size) {
    if (unlikely(dlsymUsed())) {
        return staticAlloc(nmemb * size);
    }
    static void *(*libc_calloc)(size_t nmemb, size_t size) = nullptr;
    if (unlikely(!libc_calloc)) {
        libc_calloc = (decltype(libc_calloc))dlsym(RTLD_NEXT, "calloc");
    }
    memory_hook_shared_lock();
    void *ptr = libc_calloc(nmemb, size);
    memory_hook_unlock();
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (unlikely(dlsymUsed())) {
        if (ptr) {
            hook_assert(isStaticAddr(ptr));
        }
        return staticAlloc(size);
    }
    static void *(*libc_realloc)(void *ptr, size_t size) = nullptr;
    if (unlikely(!libc_realloc)) {
        libc_realloc = (decltype(libc_realloc))dlsym(RTLD_NEXT, "realloc");
    }
    memory_hook_shared_lock();
    void *nptr = libc_realloc(ptr, size);
    memory_hook_unlock();
    return nptr;
}

void free(void *ptr) {
    if (!ptr) return;
    if (unlikely(dlsymUsed() || isStaticAddr(ptr))) {
        return;
    }
    hook_assert(!dlsymUsed());
    static void *(*libc_free)(void *ptr) = nullptr;
    if (unlikely(!libc_free)) {
        libc_free = (decltype(libc_free))dlsym(RTLD_NEXT, "free");
    }
    memory_hook_shared_lock();
    libc_free(ptr);
    memory_hook_unlock();
}

void *valloc(size_t size) {
    static void *(*libc_valloc)(size_t size) = nullptr;
    if (unlikely(!libc_valloc)) {
        libc_valloc = (decltype(libc_valloc))dlsym(RTLD_NEXT, "valloc");
    }
    memory_hook_shared_lock();
    void *ptr = libc_valloc(size);
    memory_hook_unlock();
    return ptr;
}

void *pvalloc(size_t size) {
    return valloc(size);
}

int posix_memalign(void **pptr, size_t align, size_t size) {
    static int (*libc_posix_memalign)(void **pptr, size_t align, size_t size) = nullptr;
    if (unlikely(!libc_posix_memalign)) {
        libc_posix_memalign = (decltype(libc_posix_memalign))dlsym(RTLD_NEXT, "posix_memalign");
    }
    memory_hook_shared_lock();
    int ret = libc_posix_memalign(pptr, align, size);
    memory_hook_unlock();
    return ret;
}

void *aligned_alloc(size_t align, size_t size) {
    void *ptr = nullptr;
    posix_memalign(&ptr, align, size);
    return ptr;
}

void *memalign(size_t align, size_t size) {
    return aligned_alloc(align, size);
}

__END_DECLS

void *operator new(std::size_t size);
void *operator new[](std::size_t size);
void *operator new(std::size_t size, const std::nothrow_t &) noexcept;
void *operator new[](std::size_t size, const std::nothrow_t &) noexcept;
void operator delete(void *ptr) noexcept;
void operator delete[](void *ptr) noexcept;
void operator delete(void *ptr, const std::nothrow_t &) noexcept;
void operator delete[](void *ptr, const std::nothrow_t &) noexcept;

#if __cpp_sized_deallocation >= 201309
// C++14's sized-delete operators.
void operator delete(void *ptr, std::size_t size) noexcept;
void operator delete[](void *ptr, std::size_t size) noexcept;
#endif

#if __cpp_aligned_new >= 201606
// C++17's over-aligned operators.
void *operator new(std::size_t size, std::align_val_t);
void *operator new(std::size_t size, std::align_val_t, const std::nothrow_t &) noexcept;
void *operator new[](std::size_t size, std::align_val_t);
void *operator new[](std::size_t size, std::align_val_t, const std::nothrow_t &) noexcept;
void operator delete(void *ptr, std::align_val_t) noexcept;
void operator delete(void *ptr, std::align_val_t, const std::nothrow_t &) noexcept;
void operator delete(void *ptr, std::size_t size, std::align_val_t al) noexcept;
void operator delete[](void *ptr, std::align_val_t) noexcept;
void operator delete[](void *ptr, std::align_val_t, const std::nothrow_t &) noexcept;
void operator delete[](void *ptr, std::size_t size, std::align_val_t al) noexcept;
#endif

static void *handleOOM(std::size_t size, bool nothrow) {
    void *ptr = nullptr;
    while (ptr == nullptr) {
        // GCC-4.8 and clang 4.0 do not have std::get_new_handler
        std::new_handler handler;
        {
            static std::mutex mtx;
            std::lock_guard<std::mutex> lock(mtx);
            handler = std::set_new_handler(nullptr);
            std::set_new_handler(handler);
        }
        if (handler == nullptr) {
            break;
        }
        try {
            handler();
        } catch (const std::bad_alloc &) {
            break;
        }
        ptr = malloc(size);
    }
    if (ptr == nullptr && !nothrow) {
        std::__throw_bad_alloc();
    }
    return ptr;
}

template <bool IsNoExcept>
void *newImpl(std::size_t size) noexcept(IsNoExcept) {
    void *ptr = malloc(size);
    if (likely(ptr != nullptr)) {
        return ptr;
    }
    return handleOOM(size, IsNoExcept);
}

void *operator new(std::size_t size) {
    return newImpl<false>(size);
}

void *operator new[](std::size_t size) {
    return newImpl<false>(size);
}

void *operator new(std::size_t size, const std::nothrow_t &) noexcept {
    return newImpl<true>(size);
}

void *operator new[](std::size_t size, const std::nothrow_t &) noexcept {
    return newImpl<true>(size);
}

void operator delete(void *ptr) noexcept {
    free(ptr);
}

void operator delete[](void *ptr) noexcept {
    free(ptr);
}

void operator delete(void *ptr, const std::nothrow_t &) noexcept {
    free(ptr);
}

void operator delete[](void *ptr, const std::nothrow_t &) noexcept {
    free(ptr);
}

#if __cpp_sized_deallocation >= 201309

void operator delete(void *ptr, std::size_t size) noexcept {
    free(ptr);
}

void operator delete[](void *ptr, std::size_t size) noexcept {
    free(ptr);
}

#endif // __cpp_sized_deallocation

#if __cpp_aligned_new >= 201606

template <bool IsNoExcept>
void *alignedNewImpl(std::size_t size, std::align_val_t alignment) noexcept(IsNoExcept) {
    void *ptr = aligned_alloc(static_cast<std::size_t>(alignment), size);
    if (likely(ptr)) {
        return ptr;
    }
    return handleOOM(size, IsNoExcept);
}

void *operator new(std::size_t size, std::align_val_t alignment) {
    return alignedNewImpl<false>(size, alignment);
}

void *operator new[](std::size_t size, std::align_val_t alignment) {
    return alignedNewImpl<false>(size, alignment);
}

void *operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t &) noexcept {
    return alignedNewImpl<true>(size, alignment);
}

void *operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t &) noexcept {
    return alignedNewImpl<true>(size, alignment);
}

void operator delete(void *ptr, std::align_val_t) noexcept {
    free(ptr);
}

void operator delete[](void *ptr, std::align_val_t) noexcept {
    free(ptr);
}

void operator delete(void *ptr, std::align_val_t, const std::nothrow_t &) noexcept {
    free(ptr);
}

void operator delete[](void *ptr, std::align_val_t, const std::nothrow_t &) noexcept {
    free(ptr);
}

void operator delete(void *ptr, std::size_t size, std::align_val_t alignment) noexcept {
    free(ptr);
}

void operator delete[](void *ptr, std::size_t size, std::align_val_t alignment) noexcept {
    free(ptr);
}

#endif // __cpp_aligned_new

#else

void initMemoryHookForWorkaroundASAN() {
}

#endif // defined(__linux__) && defined(USE_SANITIZER)
