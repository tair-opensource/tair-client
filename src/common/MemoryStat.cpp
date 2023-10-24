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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <libproc.h>
#include <mach/mach_init.h>
#include <mach/task.h>
#include <malloc/malloc.h> // for malloc_size()
#include <sys/sysctl.h>
#else
#include <malloc.h> // for malloc_usable_size()
#endif

#if defined(USE_JEMALLOC)
#include "jemalloc/jemalloc.h"
// Double expansion needed for stringification of macro values
#define __xstr(s)  __str(s)
#define __str(s)   #s
#define MALLOC_LIB ("jemalloc-" __xstr(JEMALLOC_VERSION_MAJOR) "." __xstr(JEMALLOC_VERSION_MINOR) "." __xstr(JEMALLOC_VERSION_BUGFIX))
#else
#define MALLOC_LIB "libc"
#endif

namespace tair::common {

size_t tair_malloc_usable_size(void *ptr) {
#if defined(__APPLE__)
    return malloc_size(ptr);
#else
    return malloc_usable_size(ptr);
#endif
}

bool tair_malloc_purge() {
#if defined(USE_JEMALLOC)
    char tmp[32];
    unsigned narenas = 0;
    size_t sz = sizeof(unsigned);
    if (!mallctl("arenas.narenas", &narenas, &sz, nullptr, 0)) {
        sprintf(tmp, "arena.%d.purge", narenas);
        if (!mallctl(tmp, nullptr, nullptr, nullptr, 0)) {
            return true;
        }
    }
    return false;
#else
    return true;
#endif
}

size_t tair_nallocx(size_t size) {
#if defined(USE_JEMALLOC) && JEMALLOC_VERSION_MAJOR >= 5
    /// The nallocx() function allocates no memory, but it performs the same size computation as the mallocx() function
    /// @note mallocx() != malloc(). It's expected they don't differ much in allocation logic.
    if (size != 0) {
        return nallocx(size, 0);
    }
#endif
    return size;
}

void tair_malloc_bg_thread(bool enable) {
#if defined(USE_JEMALLOC)
    char val = !!enable;
    mallctl("background_thread", nullptr, 0, &val, 1);
#endif
}

std::atomic<size_t> MemoryStat::system_memory_size_ = 0;

std::atomic<size_t> MemoryStat::allocator_resident_ = 0;
std::atomic<size_t> MemoryStat::allocator_active_ = 0;
std::atomic<size_t> MemoryStat::allocator_allocated_ = 0;

std::atomic<size_t> MemoryStat::initial_memory_usage_ = 0;
std::atomic<size_t> MemoryStat::peak_memory_size_ = 0;
std::atomic<size_t> MemoryStat::cached_rss_memory_size_ = 0;

const char *MemoryStat::getMallocLibName() {
    return MALLOC_LIB;
}

std::string MemoryStat::getMallocStatsInfo() {
    std::string stats;
#if defined(USE_JEMALLOC)
    auto callback = [](void *arg, const char *info) {
        std::string *stats = (std::string *)arg;
        stats->append(info);
    };
    malloc_stats_print(callback, &stats, nullptr);
#else
    stats = "Stats not supported for the current allocator";
#endif
    return stats;
}

size_t MemoryStat::mallocGetPrivateDirty(long pid) {
    return mallocGetSmapBytesByField("Private_Dirty:", pid);
}

// Get the RSS information in an OS-specific way.
//
// WARNING: the function is not designed to be fast and may not be called
// in the busy loops where tries to release memory expiring or swapping out objects.
//
size_t MemoryStat::getRssMemorySize() {
#if defined(__linux__)
    int page = sysconf(_SC_PAGESIZE);
    size_t rss;
    char buf[4096];
    char filename[256];
    int fd, count;
    char *p, *x;

    snprintf(filename, 256, "/proc/%d/stat", getpid());
    if ((fd = open(filename, O_RDONLY)) == -1) return 0;
    if (read(fd, buf, 4096) <= 0) {
        close(fd);
        return 0;
    }
    close(fd);

    p = buf;
    count = 23; // RSS is the 24th field in /proc/<pid>/stat
    while (p && count--) {
        p = strchr(p, ' ');
        if (p) p++;
    }
    if (!p) return 0;
    x = strchr(p, ' ');
    if (!x) return 0;
    *x = '\0';

    rss = strtoll(p, NULL, 10);
    rss *= page;
    return rss;
#elif defined(__APPLE__)
    task_t task = MACH_PORT_NULL;
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (task_for_pid(current_task(), getpid(), &task) != KERN_SUCCESS) {
        return 0;
    }
    task_info(task, TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
    return t_info.resident_size;
#else
    // If we can't get the RSS in an OS-specific way for this system just return the memory usage we estimated in zmalloc().
    // Fragmentation will appear to be always 1 (no fragmentation) of course...
    return getMallocUsedMemorySize();
#endif
}

void MemoryStat::mallocGetAllocatorInfo(size_t *resident, size_t *active, size_t *allocated) {
#if defined(USE_JEMALLOC)
    size_t epoch = 1;
    size_t sz;
    // Update the statistics cached by mallctl
    sz = sizeof(epoch);
    mallctl("epoch", &epoch, &sz, &epoch, sz);
    sz = sizeof(size_t);
    // Unlike RSS, this does not include RSS from shared libraries and other non heap mappings
    mallctl("stats.resident", resident, &sz, nullptr, 0);
    // Unlike resident, this doesn't not include the pages jemalloc reserves for re-use
    // (purge will clean that)
    mallctl("stats.active", active, &sz, nullptr, 0);
    // Unlike zmalloc_used_memory, this matches the stats.resident by taking into account
    // all allocations done by this process (not only jemalloc)
    mallctl("stats.allocated", allocated, &sz, nullptr, 0);
#endif
}

size_t MemoryStat::mallocGetSmapBytesByField(const char *field, long pid) {
#ifdef __linux__
    char line[1024];
    size_t bytes = 0;
    int flen = strlen(field);
    FILE *fp;
    if (pid == -1) {
        fp = fopen("/proc/self/smaps", "r");
    } else {
        char filename[128];
        snprintf(filename, sizeof(filename), "/proc/%ld/smaps", pid);
        fp = fopen(filename, "r");
    }
    if (!fp) return 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, field, flen) == 0) {
            char *p = strchr(line, 'k');
            if (p) {
                *p = '\0';
                bytes += strtol(line + flen, NULL, 10) * 1024;
            }
        }
    }
    fclose(fp);
    return bytes;
#elif defined(__APPLE__)
    struct proc_regioninfo pri;
    if (pid == -1) {
        pid = getpid();
    }
    if (proc_pidinfo(pid, PROC_PIDREGIONINFO, 0, &pri, PROC_PIDREGIONINFO_SIZE) == PROC_PIDREGIONINFO_SIZE) {
        int pagesize = getpagesize();
        if (!strcmp(field, "Private_Dirty:")) {
            return (size_t)pri.pri_pages_dirtied * pagesize;
        } else if (!strcmp(field, "Rss:")) {
            return (size_t)pri.pri_pages_resident * pagesize;
        } else if (!strcmp(field, "AnonHugePages:")) {
            return 0;
        }
    }
    return 0;
#else
    return 0;
#endif
}

// Returns the size of physical memory (RAM) in bytes.
// It looks ugly, but this is the cleanest way to achieve cross platform results.
//
// Cleaned up from:
// http://nadeausoftware.com/articles/2012/09/c_c_tip_how_get_physical_memory_size_system
//
// Note that this function:
// 1) Was released under the following CC attribution license:
//    http://creativecommons.org/licenses/by/3.0/deed.en_US.
// 2) Was originally implemented by David Robert Nadeau.
// 3) Was modified for Redis by Matt Stancliff.
// 4) This note exists in order to comply with the original license.
#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#else
#include <sys/param.h>
#include <sys/types.h>
#include <unistd.h>
#if defined(BSD)
#include <sys/sysctl.h>
#endif
#endif

size_t MemoryStat::mallocGetSystemMemorySize() {
#if defined(_WIN32) && (defined(__CYGWIN__) || defined(__CYGWIN32__))
    // Cygwin under Windows. ------------------------------------
    // New 64-bit MEMORYSTATUSEX isn't available.  Use old 32.bit
    MEMORYSTATUS status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatus(&status);
    return status.dwTotalPhys;
#elif defined(WIN32) || defined(_WIN32)
    // Windows. -------------------------------------------------
    // Use new 64-bit MEMORYSTATUSEX, not old 32-bit MEMORYSTATUS
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#else
    // UNIX variants. -------------------------------------------
    // Prefer sysctl() over sysconf() except sysctl() HW_REALMEM and HW_PHYSMEM
#if defined(CTL_HW) && (defined(HW_MEMSIZE) || defined(HW_PHYSMEM64))
    int mib[2];
    mib[0] = CTL_HW;
#if defined(HW_MEMSIZE)
    mib[1] = HW_MEMSIZE; // OSX. ---------------------
#elif defined(HW_PHYSMEM64)
    mib[1] = HW_PHYSMEM64; // NetBSD, OpenBSD. ---------
#endif
    size_t size = 0;     // 64-bit
    size_t len = sizeof(size);
    if (sysctl(mib, 2, &size, &len, nullptr, 0) == 0)
        return size;
    return 0; /* Failed? */
#elif defined(_SC_AIX_REALMEM)
    // AIX. -----------------------------------------------------
    return sysconf(_SC_AIX_REALMEM) * 1024;
#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
    // FreeBSD, Linux, OpenBSD, and Solaris. --------------------
    return size_t(sysconf(_SC_PHYS_PAGES)) * size_t(sysconf(_SC_PAGESIZE));
#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGE_SIZE)
    // Legacy. --------------------------------------------------
    return size_t(sysconf(_SC_PHYS_PAGES)) * size_t(sysconf(_SC_PAGE_SIZE));
#elif defined(CTL_HW) && (defined(HW_PHYSMEM) || defined(HW_REALMEM))
    // DragonFly BSD, FreeBSD, NetBSD, OpenBSD, and OSX. --------
    int mib[2];
    mib[0] = CTL_HW;
#if defined(HW_REALMEM)
    mib[1] = HW_REALMEM;   // FreeBSD. -----------------
#elif defined(HW_PYSMEM)
    mib[1] = HW_PHYSMEM; // Others. ------------------
#endif
    unsigned int size = 0; /* 32-bit */
    size_t len = sizeof(size);
    if (sysctl(mib, 2, &size, &len, nullptr, 0) == 0) {
        return size;
    }
    return 0; // Failed?
#endif // sysctl and sysconf variants
#endif
}

} // namespace tair::common
