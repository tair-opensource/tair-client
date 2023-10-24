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

// clang-format off
#include <cstddef>

// Default constraint for the probe arguments as operands.
#ifndef TRACING_SDT_ARG_CONSTRAINT
#define TRACING_SDT_ARG_CONSTRAINT      "nor"
#endif

// Instruction to emit for the probe.
#define TRACING_SDT_NOP                 nop

// Note section properties.
#define TRACING_SDT_NOTE_NAME           "stapsdt"
#define TRACING_SDT_NOTE_TYPE           3

// Semaphore variables are put in this section
#define TRACING_SDT_SEMAPHORE_SECTION   ".probes"

// Size of address depending on platform.
#ifdef __LP64__
#define TRACING_SDT_ASM_ADDR            .8byte
#else
#define TRACING_SDT_ASM_ADDR            .4byte
#endif

// Assembler helper Macros.
#define TRACING_SDT_S(x)                #x
#define TRACING_SDT_ASM_1(x)            TRACING_SDT_S(x) "\n"
#define TRACING_SDT_ASM_2(a, b)         TRACING_SDT_S(a) "," TRACING_SDT_S(b) "\n"
#define TRACING_SDT_ASM_3(a, b, c)      TRACING_SDT_S(a) "," TRACING_SDT_S(b) ","      \
                                     TRACING_SDT_S(c) "\n"
#define TRACING_SDT_ASM_STRING(x)       TRACING_SDT_ASM_1(.asciz TRACING_SDT_S(x))

// Helper to determine the size of an argument.
#define TRACING_SDT_IS_ARRAY_POINTER(x)  ((__builtin_classify_type(x) == 14) ||  \
                                        (__builtin_classify_type(x) == 5))
#define TRACING_SDT_ARGSIZE(x)  (TRACING_SDT_IS_ARRAY_POINTER(x)                    \
                               ? sizeof(void*)                                \
                               : sizeof(x))

// Format of each probe arguments as operand.
// Size of the arugment tagged with TRACING_SDT_Sn, with "n" constraint.
// Value of the argument tagged with TRACING_SDT_An, with configured constraint.
#define TRACING_SDT_ARG(n, x)                                                    \
  [TRACING_SDT_S##n] "n"                ((size_t)TRACING_SDT_ARGSIZE(x)),           \
  [TRACING_SDT_A##n] TRACING_SDT_ARG_CONSTRAINT (x)

// Templates to append arguments as operands.
#define TRACING_SDT_OPERANDS_0()        [__sdt_dummy] "g" (0)
#define TRACING_SDT_OPERANDS_1(_1)      TRACING_SDT_ARG(1, _1)
#define TRACING_SDT_OPERANDS_2(_1, _2)                                           \
  TRACING_SDT_OPERANDS_1(_1), TRACING_SDT_ARG(2, _2)
#define TRACING_SDT_OPERANDS_3(_1, _2, _3)                                       \
  TRACING_SDT_OPERANDS_2(_1, _2), TRACING_SDT_ARG(3, _3)
#define TRACING_SDT_OPERANDS_4(_1, _2, _3, _4)                                   \
  TRACING_SDT_OPERANDS_3(_1, _2, _3), TRACING_SDT_ARG(4, _4)
#define TRACING_SDT_OPERANDS_5(_1, _2, _3, _4, _5)                               \
  TRACING_SDT_OPERANDS_4(_1, _2, _3, _4), TRACING_SDT_ARG(5, _5)
#define TRACING_SDT_OPERANDS_6(_1, _2, _3, _4, _5, _6)                           \
  TRACING_SDT_OPERANDS_5(_1, _2, _3, _4, _5), TRACING_SDT_ARG(6, _6)
#define TRACING_SDT_OPERANDS_7(_1, _2, _3, _4, _5, _6, _7)                       \
  TRACING_SDT_OPERANDS_6(_1, _2, _3, _4, _5, _6), TRACING_SDT_ARG(7, _7)
#define TRACING_SDT_OPERANDS_8(_1, _2, _3, _4, _5, _6, _7, _8)                   \
  TRACING_SDT_OPERANDS_7(_1, _2, _3, _4, _5, _6, _7), TRACING_SDT_ARG(8, _8)
#define TRACING_SDT_OPERANDS_9(_1, _2, _3, _4, _5, _6, _7, _8, _9)               \
  TRACING_SDT_OPERANDS_8(_1, _2, _3, _4, _5, _6, _7, _8), TRACING_SDT_ARG(9, _9)

// Templates to reference the arguments from operands in note section.
#define TRACING_SDT_ARGFMT(no)        %n[TRACING_SDT_S##no]@%[TRACING_SDT_A##no]
#define TRACING_SDT_ARG_TEMPLATE_0    // No arguments
#define TRACING_SDT_ARG_TEMPLATE_1    TRACING_SDT_ARGFMT(1)
#define TRACING_SDT_ARG_TEMPLATE_2    TRACING_SDT_ARG_TEMPLATE_1 TRACING_SDT_ARGFMT(2)
#define TRACING_SDT_ARG_TEMPLATE_3    TRACING_SDT_ARG_TEMPLATE_2 TRACING_SDT_ARGFMT(3)
#define TRACING_SDT_ARG_TEMPLATE_4    TRACING_SDT_ARG_TEMPLATE_3 TRACING_SDT_ARGFMT(4)
#define TRACING_SDT_ARG_TEMPLATE_5    TRACING_SDT_ARG_TEMPLATE_4 TRACING_SDT_ARGFMT(5)
#define TRACING_SDT_ARG_TEMPLATE_6    TRACING_SDT_ARG_TEMPLATE_5 TRACING_SDT_ARGFMT(6)
#define TRACING_SDT_ARG_TEMPLATE_7    TRACING_SDT_ARG_TEMPLATE_6 TRACING_SDT_ARGFMT(7)
#define TRACING_SDT_ARG_TEMPLATE_8    TRACING_SDT_ARG_TEMPLATE_7 TRACING_SDT_ARGFMT(8)
#define TRACING_SDT_ARG_TEMPLATE_9    TRACING_SDT_ARG_TEMPLATE_8 TRACING_SDT_ARGFMT(9)

// Semaphore define, declare and probe note format

#define TRACING_SDT_SEMAPHORE(provider, name)                                    \
  tair_sdt_semaphore_##provider##_##name

#define TRACING_SDT_DEFINE_SEMAPHORE(provider, name)                             \
  extern "C" {                                                                \
    volatile unsigned short TRACING_SDT_SEMAPHORE(provider, name)                \
    __attribute__((section(TRACING_SDT_SEMAPHORE_SECTION), used)) = 0;           \
  }

#define TRACING_SDT_DECLARE_SEMAPHORE(provider, name)                            \
  extern "C" volatile unsigned short TRACING_SDT_SEMAPHORE(provider, name)

#define TRACING_SDT_SEMAPHORE_NOTE_0(provider, name)                             \
  TRACING_SDT_ASM_1(     TRACING_SDT_ASM_ADDR 0) /*No Semaphore*/                   \

#define TRACING_SDT_SEMAPHORE_NOTE_1(provider, name)                             \
  TRACING_SDT_ASM_1(TRACING_SDT_ASM_ADDR TRACING_SDT_SEMAPHORE(provider, name))

// Structure of note section for the probe.
#define TRACING_SDT_NOTE_CONTENT(provider, name, has_semaphore, arg_template)    \
  TRACING_SDT_ASM_1(990: TRACING_SDT_NOP)                                           \
  TRACING_SDT_ASM_3(     .pushsection .note.stapsdt,"","note")                   \
  TRACING_SDT_ASM_1(     .balign 4)                                              \
  TRACING_SDT_ASM_3(     .4byte 992f-991f, 994f-993f, TRACING_SDT_NOTE_TYPE)        \
  TRACING_SDT_ASM_1(991: .asciz TRACING_SDT_NOTE_NAME)                              \
  TRACING_SDT_ASM_1(992: .balign 4)                                              \
  TRACING_SDT_ASM_1(993: TRACING_SDT_ASM_ADDR 990b)                                 \
  TRACING_SDT_ASM_1(     TRACING_SDT_ASM_ADDR 0) /*Reserved for Base Address*/      \
  TRACING_SDT_SEMAPHORE_NOTE_##has_semaphore(provider, name)                     \
  TRACING_SDT_ASM_STRING(provider)                                               \
  TRACING_SDT_ASM_STRING(name)                                                   \
  TRACING_SDT_ASM_STRING(arg_template)                                           \
  TRACING_SDT_ASM_1(994: .balign 4)                                              \
  TRACING_SDT_ASM_1(     .popsection)

// Main probe Macro.
#define TRACING_SDT_PROBE(provider, name, has_semaphore, n, arglist)             \
    __asm__ __volatile__ (                                                    \
      TRACING_SDT_NOTE_CONTENT(                                                  \
        provider, name, has_semaphore, TRACING_SDT_ARG_TEMPLATE_##n)             \
      :: TRACING_SDT_OPERANDS_##n arglist                                        \
    )                                                                         \

// Helper Macros to handle variadic arguments.
#define TRACING_SDT_NARG_(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
#define TRACING_SDT_NARG(...)                                                    \
  TRACING_SDT_NARG_(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define TRACING_SDT_PROBE_N(provider, name, has_semaphore, N, ...)               \
  TRACING_SDT_PROBE(provider, name, has_semaphore, N, (__VA_ARGS__))
