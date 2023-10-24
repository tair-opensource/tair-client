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

#include <memory>

#include "common/Copyable.hpp"
#include "common/KeyHash.hpp"
#include "common/StringUtil.hpp"

namespace tair::common {

using common::KeyHash;

class SlotsBitset;
using SlotsBitsetPtr = std::shared_ptr<SlotsBitset>;

class SlotsBitset : public Copyable {
public:
    SlotsBitset() = default;
    explicit SlotsBitset(const std::bitset<KeyHash::SLOTS_NUM> &bitset)
        : bitset_(bitset) {}
    explicit SlotsBitset(const std::string &slots) {
        loadFromString(slots);
    }
    ~SlotsBitset() = default;

    void fill() {
        bitset_.set(); // set all slot to 1
    }

    bool add(uint16_t n) {
        if (n < KeyHash::SLOTS_NUM && !has(n)) {
            bitset_.set(n, true);
            return true;
        } else {
            return false;
        }
    }

    bool has(uint16_t n) const {
        if (n < KeyHash::SLOTS_NUM) {
            return bitset_[n];
        }
        return false;
    }

    bool none() {
        return bitset_.none();
    }

    bool all() {
        return bitset_.all();
    }

    bool isSlotsConflict(const SlotsBitset &that_bitset) {
        return (bitset_ & that_bitset.bitset_).any(); // if any bit is set by both bitset, return true;
    }

    SlotsBitset &operator&=(const SlotsBitset &rhs) {
        bitset_ &= rhs.bitset_;
        return *this;
    }

    SlotsBitset &operator|=(const SlotsBitset &rhs) {
        bitset_ |= rhs.bitset_;
        return *this;
    }

    SlotsBitset operator~() const {
        return SlotsBitset(~bitset_);
    }

    // example: 1,2-10,20-30,47
    bool loadFromString(const std::string &slots);
    std::string toString() const;

private:
    std::bitset<KeyHash::SLOTS_NUM> bitset_;
};

} // namespace tair::common