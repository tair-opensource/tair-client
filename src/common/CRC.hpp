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

namespace tair::common {

class CRC {
public:
    /* CRC16 implementation according to CCITT standards.
     *
     * Note by @antirez: this is actually the XMODEM CRC 16 algorithm, using the
     * following parameters:
     *
     * Name                       : "XMODEM", also known as "ZMODEM", "CRC-16/ACORN"
     * Width                      : 16 bit
     * Poly                       : 1021 (That is actually x^16 + x^12 + x^5 + 1)
     * Initialization             : 0000
     * Reflect Input byte          : False
     * Reflect Output CRC          : False
     * Xor constant to output CRC : 0000
     * Output for "123456789"     : 31C3
     */
    static uint16_t crc16(const char *buf, int len);

    /* Redis uses the CRC64 variant with "Jones" coefficients and init value of 0
     *
     * Specification of this CRC64 variant follows:
     *
     * Name                       : crc-64-jones
     * Width                      : 64 bites
     * Poly                       : 0xad93d23594c935a9
     * Reflected In                : True
     * Xor_In                     : 0xffffffffffffffff
     * Reflected_Out               : True
     * Xor_Out                    : 0x0
     * Check("123456789")         : 0xe9c6d914c4b8d9ca
     */
    static uint64_t crc64(uint64_t crc, const char *s, uint64_t l);
};

} // namespace tair::common
