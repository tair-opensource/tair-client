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
#include "common/LZF.hpp"

#include "common/Compiler.hpp"

#define STANDALONE 1 /* at the moment, this is ok. */

#ifndef STANDALONE
#include "lzf.h"
#endif

/*
 * Size of hashtable is (1 << HLOG) * sizeof (char *)
 * decompression is independent of the hash table size
 * the difference between 15 and 14 is very small
 * for small blocks (and 14 is usually a bit faster).
 * For a low-memory/faster configuration, use HLOG == 13;
 * For best compression, use 15 or 16 (or more, up to 22).
 */
#ifndef HLOG
#define HLOG 16
#endif

/*
 * Sacrifice very little compression quality in favour of compression speed.
 * This gives almost the same compression as the default code, and is
 * (very roughly) 15% faster. This is the preferred mode of operation.
 */
#ifndef VERY_FAST
#define VERY_FAST 1
#endif

/*
 * Sacrifice some more compression quality in favour of compression speed.
 * (roughly 1-2% worse compression for large blocks and
 * 9-10% for small, redundant, blocks and >>20% better speed in both cases)
 * In short: when in need for speed, enable this for binary data,
 * possibly disable this for text data.
 */
#ifndef ULTRA_FAST
#define ULTRA_FAST 0
#endif

/*
 * Unconditionally aligning does not cost very much, so do it if unsure
 */
#ifndef STRICT_ALIGN
#if !(defined(__i386) || defined(__amd64))
#define STRICT_ALIGN 1
#else
#define STRICT_ALIGN 0
#endif
#endif

/*
 * You may choose to pre-set the hash table (might be faster on some
 * modern cpus and large (>>64k) blocks, and also makes compression
 * deterministic/repeatable when the configuration otherwise is the same).
 */
#ifndef INIT_HTAB
#define INIT_HTAB 0
#endif

/*
 * Avoid assigning values to errno variable? for some embedding purposes
 * (linux kernel for example), this is necessary. NOTE: this breaks
 * the documentation in lzf.h. Avoiding errno has no speed impact.
 */
#ifndef AVOID_ERRNO
#define AVOID_ERRNO 0
#endif

/*
 * Whether to pass the LZF_STATE variable as argument, or allocate it
 * on the stack. For small-stack environments, define this to 1.
 * NOTE: this breaks the prototype in lzf.h.
 */
#ifndef LZF_STATE_ARG
#define LZF_STATE_ARG 0
#endif

/*
 * Whether to add extra checks for input validity in lzf_decompress
 * and return EINVAL if the input stream has been corrupted. This
 * only shields against overflowing the input buffer and will not
 * detect most corrupted streams.
 * This check is not normally noticeable on modern hardware
 * (<1% slowdown), but might slow down older cpus considerably.
 */
#ifndef CHECK_INPUT
#define CHECK_INPUT 1
#endif

/*
 * Whether to store pointers or offsets inside the hash table. On
 * 64 bit architectures, pointers take up twice as much space,
 * and might also be slower. Default is to autodetect.
 */
/*#define LZF_USER_OFFSETS autodetect */

/* *************************************************************************** */
/* nothing should be changed below */

#ifdef __cplusplus
#include <climits>
#include <cstring>
using namespace std;
#else
#include <limits.h>
#include <string.h>
#endif

#ifndef LZF_USE_OFFSETS
#if defined(WIN32)
#define LZF_USE_OFFSETS defined(_M_X64)
#else
#if __cplusplus > 199711L
#include <cstdint>
#else
#include <stdint.h>
#endif
#define LZF_USE_OFFSETS (UINTPTR_MAX > 0xffffffffU)
#endif
#endif

typedef unsigned char u8;

#if LZF_USE_OFFSETS
#define LZF_HSLOT_BIAS ((const u8 *)in_data)
typedef unsigned int LZF_HSLOT;
#else
#define LZF_HSLOT_BIAS 0
typedef const u8 *LZF_HSLOT;
#endif

typedef LZF_HSLOT LZF_STATE[1 << (HLOG)];

#if !STRICT_ALIGN
/* for unaligned accesses we need a 16 bit datatype. */
#if USHRT_MAX == 65535
typedef unsigned short u16;
#elif UINT_MAX == 65535
typedef unsigned int u16;
#else
#undef STRICT_ALIGN
#define STRICT_ALIGN 1
#endif
#endif

#if ULTRA_FAST
#undef VERY_FAST
#endif

namespace tair::common {

#define HSIZE (1 << (HLOG))

/*
 * don't play with this unless you benchmark!
 * the data format is not dependent on the hash function.
 * the hash function might seem strange, just believe me,
 * it works ;)
 */
#ifndef FRST
#define FRST(p)    (((p[0]) << 8) | p[1])
#define NEXT(v, p) (((v) << 8) | p[2])
#if ULTRA_FAST
#define IDX(h) (((h >> (3 * 8 - HLOG)) - h) & (HSIZE - 1))
#elif VERY_FAST
#define IDX(h) (((h >> (3 * 8 - HLOG)) - h * 5) & (HSIZE - 1))
#else
#define IDX(h) ((((h ^ (h << 5)) >> (3 * 8 - HLOG)) - h * 5) & (HSIZE - 1))
#endif
#endif
/*
 * IDX works because it is very similar to a multiplicative hash, e.g.
 * ((h * 57321 >> (3*8 - HLOG)) & (HSIZE - 1))
 * the latter is also quite fast on newer CPUs, and compresses similarly.
 *
 * the next one is also quite good, albeit slow ;)
 * (int)(cos(h & 0xffffff) * 1e6)
 */
#if 0
/* original lzv-like hash function, much worse and thus slower */
#define FRST(p)    (p[0] << 5) ^ p[1]
#define NEXT(v, p) ((v) << 5) ^ p[2]
#define IDX(h)     ((h) & (HSIZE - 1))
#endif

#define MAX_LIT (1 << 5)
#define MAX_OFF (1 << 13)
#define MAX_REF ((1 << 8) + (1 << 3))

#if __GNUC__ >= 3
#define expect(expr, value) __builtin_expect((expr), (value))
#define inline inline
#else
#define expect(expr, value) (expr)
#define inline static
#endif

#define expect_false(expr) expect((expr) != 0, 0)
#define expect_true(expr)  expect((expr) != 0, 1)

/*
 * compressed format
 *
 * 000LLLLL <L+1>    ; literal, L+1=1..33 octets
 * LLLooooo oooooooo ; backref L+1=1..7 octets, o+1=1..4096 offset
 * 111ooooo LLLLLLLL oooooooo ; backref L+8 octets, o+1=1..4096 offset
 *
 */

/*
 * Compress in_len bytes stored at the memory block starting at
 * in_data and write the result to out_data, up to a maximum length
 * of out_len bytes.
 *
 * If the output buffer is not large enough or any error occurs return 0,
 * otherwise return the number of bytes used, which might be considerably
 * more than in_len (but less than 104% of the original size), so it
 * makes sense to always use out_len == in_len - 1), to ensure _some_
 * compression, and store the data uncompressed otherwise (with a flag, of
 * course.
 *
 * lzf_compress might use different algorithms on different systems and
 * even different runs, thus might result in different compressed strings
 * depending on the phase of the moon or similar factors. However, all
 * these strings are architecture-independent and will result in the
 * original data when decompressed using lzf_decompress.
 *
 * The buffers must not be overlapping.
 *
 * If the option LZF_STATE_ARG is enabled, an extra argument must be
 * supplied which is not reflected in this header file.
 *
 */
NO_SANITIZE("alignment")
size_t LZF::lzfCompress(const void *in_data, size_t in_len, void *out_data, size_t out_len
#if LZF_STATE_ARG
                        ,
                        LZF_STATE htab
#endif
) {
#if !LZF_STATE_ARG
    LZF_STATE htab;
#endif
    const u8 *ip = (const u8 *)in_data;
    u8 *op = (u8 *)out_data;
    const u8 *in_end = ip + in_len;
    u8 *out_end = op + out_len;
    const u8 *ref;

    /* off requires a type wide enough to hold a general pointer difference.
     * ISO C doesn't have that (size_t might not be enough and ptrdiff_t only
     * works for differences within a single object). We also assume that no
     * no bit pattern traps. Since the only platform that is both non-POSIX
     * and fails to support both assumptions is windows 64 bit, we make a
     * special workaround for it.
     */
#if defined(WIN32) && defined(_M_X64)
    unsigned _int64 off; /* workaround for missing POSIX compliance */
#else
    size_t off;
#endif
    unsigned int hval;
    int lit;

    if (!in_len || !out_len)
        return 0;

#if INIT_HTAB
    memset(htab, 0, sizeof(htab));
#endif

    lit = 0;
    op++; /* start run */

    hval = FRST(ip);
    while (ip < in_end - 2) {
        LZF_HSLOT *hslot;

        hval = NEXT(hval, ip);
        hslot = htab + IDX(hval);
        ref = *hslot + LZF_HSLOT_BIAS;
        *hslot = ip - LZF_HSLOT_BIAS;

        if (1
#if INIT_HTAB
            && ref < ip /* the next test will actually take care of this, but this is faster */
#endif
            && (off = ip - ref - 1) < MAX_OFF
            && ref > (u8 *)in_data
            && ref[2] == ip[2]
#if STRICT_ALIGN
            && ((ref[1] << 8) | ref[0]) == ((ip[1] << 8) | ip[0])
#else
            && *(u16 *)ref == *(u16 *)ip
#endif
        ) {
            /* match found at *ref++ */
            size_t len = 2;
            size_t maxlen = in_end - ip - len;
            maxlen = maxlen > MAX_REF ? MAX_REF : maxlen;

            if (expect_false(op + 3 + 1 >= out_end)) /* first a faster conservative test */
                if (op - !lit + 3 + 1 >= out_end)    /* second the exact but rare test */
                    return 0;

            op[-lit - 1] = lit - 1; /* stop run */
            op -= !lit;             /* undo run if length is zero */

            for (;;) {
                if (expect_true(maxlen > 16)) {
                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;

                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;

                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;

                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;
                    len++;
                    if (ref[len] != ip[len]) break;
                }
                do {
                    len++;
                } while (len < maxlen && ref[len] == ip[len]);
                break;
            }
            len -= 2; /* len is now #octets - 1 */
            ip++;

            if (len < 7) {
                *op++ = (off >> 8) + (len << 5);
            } else {
                *op++ = (off >> 8) + (7 << 5);
                *op++ = len - 7;
            }
            *op++ = off;
            lit = 0;
            op++; /* start run */
            ip += len + 1;

            if (expect_false(ip >= in_end - 2))
                break;

#if ULTRA_FAST || VERY_FAST
            --ip;
#if VERY_FAST && !ULTRA_FAST
            --ip;
#endif
            hval = FRST(ip);

            hval = NEXT(hval, ip);
            htab[IDX(hval)] = ip - LZF_HSLOT_BIAS;
            ip++;

#if VERY_FAST && !ULTRA_FAST
            hval = NEXT(hval, ip);
            htab[IDX(hval)] = ip - LZF_HSLOT_BIAS;
            ip++;
#endif
#else
            ip -= len + 1;
            do {
                hval = NEXT(hval, ip);
                htab[IDX(hval)] = ip - LZF_HSLOT_BIAS;
                ip++;
            } while (len--);
#endif
        } else {
            /* one more literal byte we must copy */
            if (expect_false(op >= out_end))
                return 0;

            lit++;
            *op++ = *ip++;

            if (expect_false(lit == MAX_LIT)) {
                op[-lit - 1] = lit - 1; /* stop run */
                lit = 0;
                op++; /* start run */
            }
        }
    }
    if (op + 3 > out_end) /* at most 3 bytes can be missing here */
        return 0;

    while (ip < in_end) {
        lit++;
        *op++ = *ip++;
        if (expect_false(lit == MAX_LIT)) {
            op[-lit - 1] = lit - 1; /* stop run */
            lit = 0;
            op++; /* start run */
        }
    }
    op[-lit - 1] = lit - 1; /* end run */
    op -= !lit;             /* undo run if length is zero */

    return op - (u8 *)out_data;
}

#if AVOID_ERRNO
#define SET_ERRNO(n)
#else
#include <errno.h>
#define SET_ERRNO(n) errno = (n)
#endif

#if USE_REP_MOVSB /* small win on amd, big loss on intel */
#if (__i386 || __amd64) && __GNUC__ >= 3
#define lzf_movsb(dst, src, len)          \
    asm("rep movsb"                       \
        : "=D"(dst), "=S"(src), "=c"(len) \
        : "0"(dst), "1"(src), "2"(len));
#endif
#endif

#if defined(__GNUC__) && __GNUC__ == 5
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

/*
 * Decompress data compressed with some version of the lzf_compress
 * function and stored at location in_data and length in_len. The result
 * will be stored at out_data up to a maximum of out_len characters.
 *
 * If the output buffer is not large enough to hold the decompressed
 * data, a 0 is returned and errno is set to E2BIG. Otherwise the number
 * of decompressed bytes (i.e. the original length of the data) is
 * returned.
 *
 * If an error in the compressed data is detected, a zero is returned and
 * errno is set to EINVAL.
 *
 * This function is very fast, about as fast as a copying loop.
 */
size_t LZF::lzfDecompress(const void *in_data, size_t in_len, void *out_data, size_t out_len) {
    u8 const *ip = (const u8 *)in_data;
    u8 *op = (u8 *)out_data;
    u8 const *const in_end = ip + in_len;
    u8 *const out_end = op + out_len;

    do {
        unsigned int ctrl = *ip++;
        if (ctrl < (1 << 5)) { /* literal run */
            ctrl++;
            if (op + ctrl > out_end) {
                SET_ERRNO(E2BIG);
                return 0;
            }
#if CHECK_INPUT
            if (ip + ctrl > in_end) {
                SET_ERRNO(EINVAL);
                return 0;
            }
#endif
#ifdef lzf_movsb
            lzf_movsb(op, ip, ctrl);
#else
            switch (ctrl) {
                case 32: *op++ = *ip++;
                case 31: *op++ = *ip++;
                case 30: *op++ = *ip++;
                case 29: *op++ = *ip++;
                case 28: *op++ = *ip++;
                case 27: *op++ = *ip++;
                case 26: *op++ = *ip++;
                case 25: *op++ = *ip++;
                case 24: *op++ = *ip++;
                case 23: *op++ = *ip++;
                case 22: *op++ = *ip++;
                case 21: *op++ = *ip++;
                case 20: *op++ = *ip++;
                case 19: *op++ = *ip++;
                case 18: *op++ = *ip++;
                case 17: *op++ = *ip++;
                case 16: *op++ = *ip++;
                case 15: *op++ = *ip++;
                case 14: *op++ = *ip++;
                case 13: *op++ = *ip++;
                case 12: *op++ = *ip++;
                case 11: *op++ = *ip++;
                case 10: *op++ = *ip++;
                case 9: *op++ = *ip++;
                case 8: *op++ = *ip++;
                case 7: *op++ = *ip++;
                case 6: *op++ = *ip++;
                case 5: *op++ = *ip++;
                case 4: *op++ = *ip++;
                case 3: *op++ = *ip++;
                case 2: *op++ = *ip++;
                case 1: *op++ = *ip++;
            }
#endif
        } else { /* back reference */
            size_t len = ctrl >> 5;
            u8 *ref = op - ((ctrl & 0x1f) << 8) - 1;
#if CHECK_INPUT
            if (ip >= in_end) {
                SET_ERRNO(EINVAL);
                return 0;
            }
#endif
            if (len == 7) {
                len += *ip++;
#if CHECK_INPUT
                if (ip >= in_end) {
                    SET_ERRNO(EINVAL);
                    return 0;
                }
#endif
            }
            ref -= *ip++;
            if (op + len + 2 > out_end) {
                SET_ERRNO(E2BIG);
                return 0;
            }
            if (ref < (u8 *)out_data) {
                SET_ERRNO(EINVAL);
                return 0;
            }
#ifdef lzf_movsb
            len += 2;
            lzf_movsb(op, ref, len);
#else
            switch (len) {
                default:
                    len += 2;
                    if (op >= ref + len) {
                        /* disjunct areas */
                        memcpy(op, ref, len);
                        op += len;
                    } else {
                        /* overlapping, use octte by octte copying */
                        do {
                            *op++ = *ref++;
                        } while (--len);
                    }
                    break;
                case 9: *op++ = *ref++; /* fall-thru */
                case 8: *op++ = *ref++; /* fall-thru */
                case 7: *op++ = *ref++; /* fall-thru */
                case 6: *op++ = *ref++; /* fall-thru */
                case 5: *op++ = *ref++; /* fall-thru */
                case 4: *op++ = *ref++; /* fall-thru */
                case 3: *op++ = *ref++; /* fall-thru */
                case 2: *op++ = *ref++; /* fall-thru */
                case 1: *op++ = *ref++; /* fall-thru */
                case 0:
                    *op++ = *ref++; /* two octets more */
                    *op++ = *ref++; /* fall-thru */
            }
#endif
        }
    } while (ip < in_end);

    return op - (u8 *)out_data;
}

#if defined(__GNUC__) && __GNUC__ >= 5
#pragma GCC diagnostic pop
#endif

} // namespace tair::common
