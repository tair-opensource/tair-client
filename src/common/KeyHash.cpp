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
#include "common/KeyHash.hpp"

#include "common/CRC.hpp"

namespace tair::common {

uint16_t KeyHash::keyHashSlot(const char *key, size_t keylen) {
    size_t s = 0, e = 0; // start-end indexes of { and }
    for (s = 0; s < keylen; s++) {
        if (key[s] == '{') {
            break;
        }
    }
    // No '{' ? Hash the whole key. This is the base case
    if (s == keylen) {
        return CRC::crc16(key, keylen) & SLOTS_NUM_MASK;
    }
    // '{' found? Check if we have the corresponding '}'
    for (e = s + 1; e < keylen; e++) {
        if (key[e] == '}') {
            break;
        }
    }
    // No '}' or nothing between {} ? Hash the whole key
    if (e == keylen || e == s + 1) {
        return CRC::crc16(key, keylen) & SLOTS_NUM_MASK;
    }
    // If we are here there is both a { and a } on its right. Hash what is in the middle between { and }
    return CRC::crc16(key + s + 1, e - s - 1) & SLOTS_NUM_MASK;
}

} // namespace tair::common
