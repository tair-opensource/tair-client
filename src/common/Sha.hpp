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

#include <cstddef>
#include <cstdint>

#define SHA256_BLOCK_SIZE 32 // SHA256 outputs a 32 byte digest

namespace tair::common {

typedef uint8_t BYTE;  // 8-bit byte
typedef uint32_t WORD; // 32-bit word

typedef struct {
    BYTE data[64];
    WORD datalen;
    unsigned long long bitlen;
    WORD state[8];
} SHA256_CTX;

class Sha256 {
public:
    static void init(SHA256_CTX *ctx);
    static void update(SHA256_CTX *ctx, const BYTE *data, size_t len);
    static void final(SHA256_CTX *ctx, BYTE *hash);
};

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

class Sha1 {
public:
    static void init(SHA1_CTX *context);
    static void update(SHA1_CTX *context, const unsigned char *data, uint32_t len);
    static void final(unsigned char digest[20], SHA1_CTX *context);
    static void transform(uint32_t state[5], const unsigned char buffer[64]);

    // Perform the SHA1 of the input string. We use this both for hashing script bodies
    // in order to obtain the Lua function name, and in the implementation of redis.sha1().
    // 'digest' should point to a 41 bytes buffer: 40 for SHA1 converted into an
    // hexadecimal number, plus 1 byte for null term.
    static void sha1hex(char *digest, const char *script, size_t len);
};

class Digest {
public:
    static void xorDigest(unsigned char digest[20], const void *ptr, size_t len);
    static void mixDigest(unsigned char digest[20], const void *ptr, size_t len);
};

} // namespace tair::common
