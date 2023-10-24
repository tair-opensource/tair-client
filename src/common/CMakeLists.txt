set(SOURCE_FILES_COMMON
    tracing/StaticTracepoint.hpp
    tracing/StaticTracepoint-ELFx86.hpp
    statistics/AtomicStatistics.hpp
    statistics/LatencyStatistics.hpp
    Assert.cpp Assert.hpp
    ConcurrentHashMap.hpp
    CRC.cpp CRC.hpp
    KeyHash.cpp KeyHash.hpp
    Logger.cpp Logger.hpp
    BlockingQueue.hpp
    BoundedQueue.hpp
    Copyable.hpp
    MapUtil.hpp
    Noncopyable.hpp
    Mutex.hpp
    Compiler.hpp
    CidrTrieTree.cpp CidrTrieTree.hpp
    CoroutineThread.cpp CoroutineThread.hpp
    ThreadPool.cpp ThreadPool.hpp
    TimeInterval.hpp
    ThreadLocal.hpp ThreadLocalSingleton.hpp
    WeakCallback.hpp
    CountDownLatch.hpp EnableMakeShared.hpp
    Endianconv.cpp Endianconv.hpp
    LZF.cpp LZF.hpp
    ClockTime.hpp
    StringUtil.cpp StringUtil.hpp
    DateTime.hpp
    SystemUtil.cpp SystemUtil.hpp
    SipHash.cpp SipHash.hpp
    LRUMap.hpp
    Sha.cpp Sha.hpp
    MathUtil.cpp MathUtil.hpp
    CPUStatistics.cpp CPUStatistics.hpp
    Stralgo.cpp Stralgo.hpp
    TokenBucket.cpp TokenBucket.hpp
    Base64.hpp Base64.cpp
    MemoryStat.cpp MemoryStat.hpp
    Singleton.hpp
    ThreadExecutor.cpp ThreadExecutor.hpp
    ThreadExecutorPool.cpp ThreadExecutorPool.hpp
    libaco/aco.c libaco/aco.h libaco/aco_assert_override.h libaco/acosw.S
    SlotsBitset.cpp SlotsBitset.hpp
)

set_property(SOURCE libaco/acosw.S PROPERTY LANGUAGE C)

add_library(tair-common ${SOURCE_FILES_COMMON})
target_link_libraries(tair-common
    absl::strings
    absl::stacktrace
    absl::symbolize
    absl::flat_hash_map
    absl::flat_hash_set
    fmt::fmt
    spdlog::spdlog
    sasl2
    ${PROJECT_SOURCE_DIR}/deps/openssl/lib/libssl.a
    ${PROJECT_SOURCE_DIR}/deps/openssl/lib/libcrypto.a)

if(MALLOC MATCHES "jemalloc")
    target_link_libraries(tair-common ${PROJECT_SOURCE_DIR}/deps/jemalloc/lib/libjemalloc_pic.a)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(tair-common dl)
endif()

add_library(asan-memory-hook
    MemoryHook.cpp MemoryHook.hpp
)

add_library(shared-library-hook SharedLibraryHook.cpp)