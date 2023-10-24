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
#include "common/Sha.hpp"

#include <cstdlib>
#include <cstring>

// clang-format off
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

namespace tair::common {

static const WORD k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_transform(SHA256_CTX *ctx, const BYTE data[]) {
    WORD a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for (; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void Sha256::init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

void Sha256::update(SHA256_CTX *ctx, const BYTE *data, size_t len) {
    WORD i;

    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void Sha256::final(SHA256_CTX *ctx, BYTE *hash) {
    WORD i = ctx->datalen;

    // Pad whatever data is left in the buffer.
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56)
            ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64)
            ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);

    // Since this implementation uses little endian byte ordering and SHA uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i) {
        hash[i] = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4] = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8] = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

void Sha1::init(SHA1_CTX *context) {
    // SHA1 initialization constants
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

void Sha1::update(SHA1_CTX *context, const unsigned char *data, uint32_t len) {
    uint32_t i, j;

    j = context->count[0];
    if ((context->count[0] += len << 3) < j)
        context->count[1]++;
    context->count[1] += (len >> 29);
    j = (j >> 3) & 63;
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        transform(context->state, context->buffer);
        for (; i + 63 < len; i += 64) {
            transform(context->state, &data[i]);
        }
        j = 0;
    } else
        i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}

void Sha1::final(unsigned char digest[20], SHA1_CTX *context) {
    unsigned i;
    unsigned char finalcount[8];
    unsigned char c;

#if 0 /* untested "improvement" by DHR */
    /* Convert context->count to a sequence of bytes
     * in finalcount.  Second element first, but
     * big-endian order within element.
     * But we do it all backwards.
     */
    unsigned char *fcp = &finalcount[8];

    for (i = 0; i < 2; i++)
       {
        uint32_t t = context->count[i];
        int j;

        for (j = 0; j < 4; t >>= 8, j++)
	          *--fcp = (unsigned char) t;
    }
#else
    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
                >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }
#endif
    c = 0200;
    update(context, &c, 1);
    while ((context->count[0] & 504) != 448) {
        c = 0000;
        update(context, &c, 1);
    }
    update(context, finalcount, 8); // Should cause a SHA1Transform()
    for (i = 0; i < 20; i++) {
        digest[i] = (unsigned char)((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }
    // Wipe variables
    memset(context, '\0', sizeof(*context));
    memset(&finalcount, '\0', sizeof(finalcount));
}

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

// blk0() and blk() perform the initial expand.
// I got the idea of expanding during the round function from SSLeay
#if BYTE_ORDER == LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#elif BYTE_ORDER == BIG_ENDIAN
#define blk0(i) block->l[i]
#else
#error "Endianness not defined!"
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

// (R0+R1), R2, R3, R4 are the different operations used in SHA1
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

#define SHA1HANDSOFF

void Sha1::transform(uint32_t *state, const unsigned char buffer[64]) {
    uint32_t a, b, c, d, e;
    typedef union {
        unsigned char c[64];
        uint32_t l[16];
    } CHAR64LONG16;
#ifdef SHA1HANDSOFF
    CHAR64LONG16 block[1]; /* use array to appear as a pointer */
    memcpy(block, buffer, 64);
#else
    // The following had better never be used because it causes the pointer-to-const buffer
    // to be cast into a pointer to non-const.
    // And the result is written through.  I threw a "const" in, hoping this will cause a diagnostic.
    CHAR64LONG16 *block = (CHAR64LONG16 *)buffer;
#endif
    // Copy context->state[] to working vars
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    // 4 rounds of 20 operations each. Loop unrolled.
    R0(a,b,c,d,e, 0) R0(e,a,b,c,d, 1) R0(d,e,a,b,c, 2) R0(c,d,e,a,b, 3)
    R0(b,c,d,e,a, 4) R0(a,b,c,d,e, 5) R0(e,a,b,c,d, 6) R0(d,e,a,b,c, 7)
    R0(c,d,e,a,b, 8) R0(b,c,d,e,a, 9) R0(a,b,c,d,e,10) R0(e,a,b,c,d,11)
    R0(d,e,a,b,c,12) R0(c,d,e,a,b,13) R0(b,c,d,e,a,14) R0(a,b,c,d,e,15)
    R1(e,a,b,c,d,16) R1(d,e,a,b,c,17) R1(c,d,e,a,b,18) R1(b,c,d,e,a,19)
    R2(a,b,c,d,e,20) R2(e,a,b,c,d,21) R2(d,e,a,b,c,22) R2(c,d,e,a,b,23)
    R2(b,c,d,e,a,24) R2(a,b,c,d,e,25) R2(e,a,b,c,d,26) R2(d,e,a,b,c,27)
    R2(c,d,e,a,b,28) R2(b,c,d,e,a,29) R2(a,b,c,d,e,30) R2(e,a,b,c,d,31)
    R2(d,e,a,b,c,32) R2(c,d,e,a,b,33) R2(b,c,d,e,a,34) R2(a,b,c,d,e,35)
    R2(e,a,b,c,d,36) R2(d,e,a,b,c,37) R2(c,d,e,a,b,38) R2(b,c,d,e,a,39)
    R3(a,b,c,d,e,40) R3(e,a,b,c,d,41) R3(d,e,a,b,c,42) R3(c,d,e,a,b,43)
    R3(b,c,d,e,a,44) R3(a,b,c,d,e,45) R3(e,a,b,c,d,46) R3(d,e,a,b,c,47)
    R3(c,d,e,a,b,48) R3(b,c,d,e,a,49) R3(a,b,c,d,e,50) R3(e,a,b,c,d,51)
    R3(d,e,a,b,c,52) R3(c,d,e,a,b,53) R3(b,c,d,e,a,54) R3(a,b,c,d,e,55)
    R3(e,a,b,c,d,56) R3(d,e,a,b,c,57) R3(c,d,e,a,b,58) R3(b,c,d,e,a,59)
    R4(a,b,c,d,e,60) R4(e,a,b,c,d,61) R4(d,e,a,b,c,62) R4(c,d,e,a,b,63)
    R4(b,c,d,e,a,64) R4(a,b,c,d,e,65) R4(e,a,b,c,d,66) R4(d,e,a,b,c,67)
    R4(c,d,e,a,b,68) R4(b,c,d,e,a,69) R4(a,b,c,d,e,70) R4(e,a,b,c,d,71)
    R4(d,e,a,b,c,72) R4(c,d,e,a,b,73) R4(b,c,d,e,a,74) R4(a,b,c,d,e,75)
    R4(e,a,b,c,d,76) R4(d,e,a,b,c,77) R4(c,d,e,a,b,78) R4(b,c,d,e,a,79)
    // Add the working vars back into context.state[]
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    // Wipe variables
    a = b = c = d = e = 0;
#ifdef SHA1HANDSOFF
    memset(block, '\0', sizeof(block));
#endif
}
// clang-format on

void Sha1::sha1hex(char *digest, const char *script, size_t len) {
    SHA1_CTX ctx;
    unsigned char hash[20];
    const char *const cset = "0123456789abcdef";

    Sha1::init(&ctx);
    Sha1::update(&ctx, (unsigned char *)script, len);
    Sha1::final(hash, &ctx);

    for (int i = 0; i < 20; i++) {
        digest[i * 2] = cset[((hash[i] & 0xF0) >> 4)];
        digest[i * 2 + 1] = cset[(hash[i] & 0xF)];
    }
    digest[40] = '\0';
}

void Digest::xorDigest(unsigned char digest[20], const void *ptr, size_t len) {
    unsigned char hash[20];

    SHA1_CTX ctx;
    Sha1::init(&ctx);
    Sha1::update(&ctx, (const unsigned char *)ptr, len);
    Sha1::final(hash, &ctx);

    for (int i = 0; i < 20; i++) {
        digest[i] ^= hash[i];
    }
}

void Digest::mixDigest(unsigned char digest[20], const void *ptr, size_t len) {
    SHA1_CTX ctx;
    xorDigest(digest, ptr, len);
    Sha1::init(&ctx);
    Sha1::update(&ctx, digest, 20);
    Sha1::final(digest, &ctx);
}

} // namespace tair::common
