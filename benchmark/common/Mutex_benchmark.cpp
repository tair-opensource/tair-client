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

#include "common/Mutex.hpp"

#include "absl/synchronization/mutex.h"

tair::common::Mutex std_mutex;
size_t a = 0;

static void BM_std_mutex(benchmark::State &state) {
    for (auto _ : state) {
        std_mutex.lock();
        for (int i = 0; i < 10000; i++) {
            a++;
        }
        std_mutex.unlock();
    }
}
BENCHMARK(BM_std_mutex)->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(16);

absl::Mutex absl_mutex;
size_t b = 0;

static void BM_absl_mutex(benchmark::State &state) {
    for (auto _ : state) {
        absl_mutex.Lock();
        for (int i = 0; i < 10000; i++) {
            b++;
        }
        absl_mutex.Unlock();
    }
}
BENCHMARK(BM_absl_mutex)->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(16);

BENCHMARK_MAIN();
