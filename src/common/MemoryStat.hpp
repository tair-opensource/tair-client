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

#include <atomic>
#include <cstddef>
#include <string>

#include "common/Noncopyable.hpp"

namespace tair::common {

size_t tair_malloc_usable_size(void *ptr);
bool tair_malloc_purge();
void tair_malloc_bg_thread(bool enable);
size_t tair_nallocx(size_t size);

class MemoryStat : private Noncopyable {
public:
    // Call in cron per 100ms
    static void calcStatistics() {
        // System memory size in ECS may changed
        system_memory_size_ = mallocGetSystemMemorySize();

        // Update used memory size and rss size
        cached_rss_memory_size_ = getRssMemorySize();

        size_t resident = 0, active = 0, allocated = 0;
        mallocGetAllocatorInfo(&resident, &active, &allocated);

        // In case the allocator isn't providing these stats,
        // fake them so that fragment info still shows some (inaccurate metrics)
        if (resident != 0) {
            allocator_resident_ = resident;
        } else {
            allocator_resident_ = cached_rss_memory_size_.load();
        }
        if (active != 0) {
            allocator_active_ = active;
        } else {
            allocator_active_ = allocator_resident_.load();
        }
        if (allocated != 0) {
            allocator_allocated_ = allocated;
        } else {
            allocator_allocated_ = allocator_active_.load();
        }
        if (peak_memory_size_ < allocator_allocated_) {
            peak_memory_size_ = allocator_allocated_.load();
        }
    }

    static inline void calcInitialMemoryUsage() {
        calcStatistics();
        initial_memory_usage_ = allocator_allocated_.load();
    }

    static inline size_t getSystemMemorySize() { return system_memory_size_; }

    static inline size_t getInitialMemoryUsage() { return initial_memory_usage_; }
    static inline size_t getPeakMemorySize() { return peak_memory_size_; }

    static inline size_t getAllocatorResident() { return allocator_resident_; }
    static inline size_t getAllocatorActive() { return allocator_active_; }
    static inline size_t getAllocatorAllocated() { return allocator_allocated_; }

    static inline size_t getCachedRssMemorySize() { return cached_rss_memory_size_; }
    static inline size_t getCachedMallocUsedMemorySize() { return allocator_allocated_; }

    static const char *getMallocLibName();
    static std::string getMallocStatsInfo();
    static size_t mallocGetPrivateDirty(long pid);
    static void mallocGetAllocatorInfo(size_t *resident, size_t *active, size_t *allocated);

    static size_t getRssMemorySize();
    static size_t mallocGetSmapBytesByField(const char *field, long pid);
    static size_t mallocGetSystemMemorySize();

private:
    static std::atomic<size_t> system_memory_size_;

    static std::atomic<size_t> allocator_resident_;
    static std::atomic<size_t> allocator_active_;
    static std::atomic<size_t> allocator_allocated_;

    static std::atomic<size_t> initial_memory_usage_;
    static std::atomic<size_t> peak_memory_size_;
    static std::atomic<size_t> cached_rss_memory_size_;
};

} // namespace tair::common
