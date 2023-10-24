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

#include <bitset>
#include <string_view>

namespace tair::common {

class KeyHash {
public:
    static const constexpr char *FULL_SLOTS = "0-16383";
    static const constexpr uint16_t SLOTS_NUM = 16384;
    static const constexpr uint16_t SLOTS_NUM_MASK = 0x3FFF;

    static const constexpr uint16_t SLOT_ID_NO_KEY = SLOTS_NUM;
    static const constexpr uint16_t SLOT_ID_CROSS = SLOTS_NUM + 1;
    static const constexpr uint16_t SLOT_ID_INVALID = UINT16_MAX;

    /* We have 16384 hash slots. The hash slot of a given key is obtained
     * as the least significant 14 bits of the crc16 of the key.
     *
     * However if the key contains the {...} pattern, only the part between
     * { and } is hashed. This may be useful in the future to force certain
     * keys to be in the same node (assuming no resharding is in progress). */
    static uint16_t keyHashSlot(const char *key, size_t keylen);

    static uint16_t keyHashSlot(const std::string &key) {
        return keyHashSlot(key.data(), key.size());
    }

    static uint16_t keyHashSlot(const std::string_view &key) {
        return keyHashSlot(key.data(), key.size());
    }
};

} // namespace tair::common
