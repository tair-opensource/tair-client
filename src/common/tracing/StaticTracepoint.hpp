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
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// copy from https://github.com/facebook/folly

#pragma once

#if defined(__ELF__) && (defined(__x86_64__) || defined(__i386__)) && !TAIR_DISABLE_SDT

#include "StaticTracepoint-ELFx86.hpp"

#define TRACING_SDT(provider, name, ...) \
    TRACING_SDT_PROBE_N(                 \
        provider, name, 0, TRACING_SDT_NARG(0, ##__VA_ARGS__), ##__VA_ARGS__)
// Use TRACING_SDT_DEFINE_SEMAPHORE(provider, name) to define the semaphore
// as global variable before using the TRACING_SDT_WITH_SEMAPHORE macro
#define TRACING_SDT_WITH_SEMAPHORE(provider, name, ...) \
    TRACING_SDT_PROBE_N(                                \
        provider, name, 1, TRACING_SDT_NARG(0, ##__VA_ARGS__), ##__VA_ARGS__)
#define TRACING_SDT_IS_ENABLED(provider, name) \
    (TRACING_SDT_SEMAPHORE(provider, name) > 0)

#else

#define TRACING_SDT(provider, name, ...) \
    do {                                 \
    } while (0)
#define TRACING_SDT_WITH_SEMAPHORE(provider, name, ...) \
    do {                                                \
    } while (0)
#define TRACING_SDT_IS_ENABLED(provider, name) (false)
#define TRACING_SDT_DEFINE_SEMAPHORE(provider, name)
#define TRACING_SDT_DECLARE_SEMAPHORE(provider, name)
#endif
