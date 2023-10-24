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
#include "common/ThreadExecutor.hpp"

#include "benchmark/benchmark.h"

static void BM_thread(benchmark::State &state) {
    int sleep = state.range(0);
    auto thread = ::tair::common::ThreadExecutor(1024);
    thread.start();
    int x = 0;
    for (auto _ : state) {
        thread.putTask([&x, sleep]() {
            x++;
            if (sleep) {
                std::this_thread::sleep_for(std::chrono::microseconds(sleep));
            }
        });
        x--;
    }
    thread.stop();
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_thread)->Arg(0)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_MAIN();
