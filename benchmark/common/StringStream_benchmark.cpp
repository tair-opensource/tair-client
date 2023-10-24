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

#include <string>
#include <sstream>

#include "benchmark/benchmark.h"
#include "fmt/format.h"

std::string create_stream(int count) {
    std::stringstream stream;
    for (int i = 0; i < count; ++i) {
        stream << "test...";
    }
    return stream.str();
}

static void BM_Stream(benchmark::State &state) {
    for (auto _ : state) {
        create_stream(state.range(0));
    }
}
BENCHMARK(BM_Stream)->Arg(10)->Arg(100)->Arg(1000);

std::string create_append(int count) {
    std::string str;
    for (int i = 0; i < count; ++i) {
        str.append("test...");
    }
    return str;
}

static void BM_Append(benchmark::State &state) {
    for (auto _ : state) {
        create_append(state.range(0));
    }
}
BENCHMARK(BM_Append)->Arg(10)->Arg(100)->Arg(1000);

std::string create_fmtbuffer(int count) {
    fmt::memory_buffer out;
    for (int i = 0; i < count; ++i) {
        fmt::format_to(std::back_inserter(out), "test...");
    }
    return fmt::to_string(out);
}

static void BM_fmtbuffer(benchmark::State &state) {
    for (auto _ : state) {
        create_fmtbuffer(state.range(0));
    }
}
BENCHMARK(BM_fmtbuffer)->Arg(10)->Arg(100)->Arg(1000);

BENCHMARK_MAIN();
