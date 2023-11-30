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
#include "common/Assert.hpp"

#include <cstdarg>

#include "common/Logger.hpp"

#include "absl/debugging/stacktrace.h"
#include "absl/debugging/symbolize.h"

std::atomic<int> tair_runtime_assert_enabled = 1;
void _runtimeAssert(const char *estr, const char *file, int line) {
    if (!tair_runtime_assert_enabled) {
        return;
    }
    LOG_ERROR("=== TAIR BUG REPORT START ===");
    LOG_ERROR("------------------------------------------------");
    LOG_ERROR("ASSERTION FAILED: {}:{} '{}' is not true", file, line, estr);
    LOG_ERROR("Backtrace:");
    logStackTrace(1);
    LOG_ERROR("------------------------------------------------");
    tair::common::Logger::instance().flushAllLogAndWait();
    *((int *)-1) = 0;
}

void _runtimePanic(const char *file, int line, const char *msg, ...) {
    if (!tair_runtime_assert_enabled) {
        return;
    }
    va_list ap;
    va_start(ap, msg);
    char fmtmsg[256];
    vsnprintf(fmtmsg, sizeof(fmtmsg), msg, ap);
    va_end(ap);

    LOG_ERROR("=== TAIR BUG REPORT START ===");
    LOG_ERROR("------------------------------------------------");
    LOG_ERROR("!!! Software Failure. Press left mouse button to continue");
    LOG_ERROR("Guru Meditation: {} #{}:{}", fmtmsg, file, line);
    LOG_ERROR("------------------------------------------------");
    LOG_ERROR("Backtrace:");
    logStackTrace(1);
    tair::common::Logger::instance().flushAllLogAndWait();
    *((int *)-1) = 0;
}

void logStackTrace(int skip_count) {
    void *results[20];
    char frame[128];
    int ret = absl::GetStackTrace(results, 20, skip_count);
    for (int i = 0; i < ret; ++i) {
        if (absl::Symbolize(results[i], frame, sizeof(frame))) {
            LOG_ERROR("{}[{}]", frame, results[i]);
        }
    }
}
