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

#include "common/Assert.hpp"
#include "common/SlotsBitset.hpp"

using tair::common::SlotsBitset;

static void BM_SlotsBitset_tostring(benchmark::State &state) {
    SlotsBitset slots("0-16383");
    for (auto _ : state) {
        slots.toString();
    }
}
BENCHMARK(BM_SlotsBitset_tostring);

static void BM_SlotsBitset_range(benchmark::State &state) {
    SlotsBitset slots("0-16383");
    for (auto _ : state) {
        size_t count = 0;
        for (uint16_t i = 0; i < 16384; ++i) {
            if (slots.has(i)) {
                count++;
            }
        }
        runtimeAssert(count == 16384);
    }
}
BENCHMARK(BM_SlotsBitset_range);

static void BM_Vector_range(benchmark::State &state) {
    std::vector<uint16_t> slots;
    slots.resize(16384, 1);
    for (auto _ : state) {
        size_t count = 0;
        for (auto slot : slots) {
            if (slot) {
                count++;
            }
        }
        runtimeAssert(count == 16384);
    }
}
BENCHMARK(BM_Vector_range);

#include <deque>

static void BM_Deque_range(benchmark::State &state) {
    std::deque<uint16_t> slots;
    slots.resize(16384, 1);
    for (auto _ : state) {
        size_t count = 0;
        for (auto slot : slots) {
            if (slot) {
                count++;
            }
        }
        runtimeAssert(count == 16384);
    }
}
BENCHMARK(BM_Deque_range);

BENCHMARK_MAIN();
