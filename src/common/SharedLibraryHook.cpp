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
#if defined(__linux__) && defined(USE_SANITIZER)
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h> // for dlsym()

#include "common/Compiler.hpp"

__BEGIN_DECLS

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

void *getcwd(char *__buf, size_t __size) {
    if (__buf == nullptr && __size == 0) {
        __buf = (char *)malloc(PATH_MAX);
        __size = PATH_MAX;
    }

    static void *(*libc_getcwd)(char *__buf, size_t __size) = nullptr;
    if (unlikely(!libc_getcwd)) {
        libc_getcwd = (decltype(libc_getcwd))dlsym(RTLD_NEXT, "getcwd");
    }
    return libc_getcwd(__buf, __size);
}

__END_DECLS

#endif