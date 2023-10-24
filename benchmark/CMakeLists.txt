set(BENCHMARK_LIB benchmark::benchmark)

add_executable(clock_time_benchmark common/ClockTime_benchmark.cpp)
target_link_libraries(clock_time_benchmark tair-common ${BENCHMARK_LIB})

add_executable(memory_stat_benchmark common/MemoryStat_benchmark.cpp)
target_link_libraries(memory_stat_benchmark tair-common ${BENCHMARK_LIB})

add_executable(mutex_benchmark common/Mutex_benchmark.cpp)
target_link_libraries(mutex_benchmark tair-common absl::synchronization ${BENCHMARK_LIB})

add_executable(num2str_benchmark common/Num2Str_benchmark.cpp)
target_link_libraries(num2str_benchmark tair-common ${BENCHMARK_LIB})

add_executable(slotsbitset_benchmark common/SlotsBitset_benchmark.cpp)
target_link_libraries(slotsbitset_benchmark tair-common ${BENCHMARK_LIB})

add_executable(statistics_benchmark common/Statistics_benchmark.cpp)
target_link_libraries(statistics_benchmark tair-common ${BENCHMARK_LIB})

add_executable(stringstream_benchmark common/StringStream_benchmark.cpp)
target_link_libraries(stringstream_benchmark tair-common ${BENCHMARK_LIB})

add_executable(thread_executor_benchmark common/ThreadExecutor_benchmark.cpp)
target_link_libraries(thread_executor_benchmark tair-common ${BENCHMARK_LIB})

add_executable(thread_pool_benchmark common/ThreadPool_benchmark.cpp)
target_link_libraries(thread_pool_benchmark tair-common ${BENCHMARK_LIB})
