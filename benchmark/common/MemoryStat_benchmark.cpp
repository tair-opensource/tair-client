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

#include "benchmark/benchmark.h"

#include <cstring>

#include "common/MemoryStat.hpp"

using tair::common::MemoryStat;

static void BM_MemoryStat(benchmark::State &state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(MemoryStat::mallocGetPrivateDirty(-1));
    }
}
BENCHMARK(BM_MemoryStat);

static void BM_MemoryStat2(benchmark::State &state) {
    auto mem = new char[1024ll * 1024 * 1024 * 10]; // 10G
    ::memset(mem, 0, 1024ll * 1024 * 1024 * 10);
    for (auto _ : state) {
        benchmark::DoNotOptimize(MemoryStat::mallocGetPrivateDirty(-1));
    }
}
BENCHMARK(BM_MemoryStat2);

BENCHMARK_MAIN();
