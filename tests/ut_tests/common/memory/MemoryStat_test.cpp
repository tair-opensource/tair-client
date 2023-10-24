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
#include "common/MemoryStat.hpp"
#include "gtest/gtest.h"

using tair::common::MemoryStat;

TEST(MEMORY_STAT_TEST, ONLY_TEST) {
#if 0
#if defined(__linux__) && defined(USE_JEMALLOC)
    MemoryStat::mallocGetAllocatorInfo();
    size_t rssBytesBefore = MemoryStat::getAllocatorResident();
    size_t privateDirtyBytesBefore = MemoryStat::mallocGetPrivateDirty(-1);
    int *number = new int[10241];
    // More than ten 4KB page table entries.
    for (size_t index = 0; index < 10241; index++) {
        number[index] = 0b1001100010011000100000111;
    }
    MemoryStat::mallocGetAllocatorInfo();
    size_t rssBytesAfter = MemoryStat::getAllocatorResident();
    size_t privateDirtyBytesAfter = MemoryStat::mallocGetPrivateDirty(getpid());
    delete[] number;
    ASSERT_LT(privateDirtyBytesBefore, privateDirtyBytesAfter);
    ASSERT_LT(rssBytesBefore, rssBytesAfter);

    ASSERT_TRUE(tair::common::tair_malloc_purge());
    number = new int[10241];
    for (size_t index = 0; index < 10241; index++) {
        number[index] = 0b1001100010011000100000111;
    }
    delete[] number;
    rssBytesBefore = MemoryStat::getRssMemorySize();
    // jemalloc purge actually uses madvise(MADV_DONTNEED) and munmap for memory cleaning.
    ASSERT_TRUE(tair::common::tair_malloc_purge());
    rssBytesAfter = MemoryStat::getRssMemorySize();
    ASSERT_LE(rssBytesAfter, rssBytesBefore);
#endif
#endif
}
