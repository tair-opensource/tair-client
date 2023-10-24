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
#pragma once

#include <cstdint>

#include <sys/types.h> // This will likely define BYTE_ORDER

#if !defined(BYTE_ORDER) || (BYTE_ORDER != BIG_ENDIAN && BYTE_ORDER != LITTLE_ENDIAN)
/* you must determine what the correct bit order is for
 * your compiler - the next line is an intentional error
 * which will force your compiles to bomb until you fix
 * the above macros.
 */
#error "Undefined or invalid BYTE_ORDER"
#endif

/* variants of the function doing the actual conversion only if the target
 * host is big endian */
#if (BYTE_ORDER == LITTLE_ENDIAN)
#define memrev16ifbe(p) ((void)(0))
#define memrev32ifbe(p) ((void)(0))
#define memrev64ifbe(p) ((void)(0))
#define intrev16ifbe(v) (v)
#define intrev32ifbe(v) (v)
#define intrev64ifbe(v) (v)
#else
#define memrev16ifbe(p) common::Endianconv::memrev16(p)
#define memrev32ifbe(p) common::Endianconv::memrev32(p)
#define memrev64ifbe(p) common::Endianconv::memrev64(p)
#define intrev16ifbe(v) common::Endianconv::intrev16(v)
#define intrev32ifbe(v) common::Endianconv::intrev32(v)
#define intrev64ifbe(v) common::Endianconv::intrev64(v)
#endif

/* The functions htonu64() and ntohu64() convert the specified value to
 * network byte ordering and back. In big endian systems they are no-ops. */
#if (BYTE_ORDER == BIG_ENDIAN)
#define htonu16(v) (v)
#define ntohu16(v) (v)
#define htonu32(v) (v)
#define ntohu32(v) (v)
#define htonu64(v) (v)
#define ntohu64(v) (v)
#else
#define htonu16(v) common::Endianconv::intrev16(v)
#define ntohu16(v) common::Endianconv::intrev16(v)
#define htonu32(v) common::Endianconv::intrev32(v)
#define ntohu32(v) common::Endianconv::intrev32(v)
#define htonu64(v) common::Endianconv::intrev64(v)
#define ntohu64(v) common::Endianconv::intrev64(v)
#endif

namespace tair::common {

class Endianconv {
public:
    static void memrev16(void *p);
    static void memrev32(void *p);
    static void memrev64(void *p);
    static uint16_t intrev16(uint16_t v);
    static uint32_t intrev32(uint32_t v);
    static uint64_t intrev64(uint64_t v);
};

} // namespace tair::common
