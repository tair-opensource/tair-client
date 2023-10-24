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
#include "common/SlotsBitset.hpp"

namespace tair::common {

// example: 1,2-10,20-30,47
bool SlotsBitset::loadFromString(const std::string &slots) {
    if (slots.empty()) {
        return false;
    }
    std::bitset<KeyHash::SLOTS_NUM> tmp_bitset {};
    auto segments = StringUtil::split(slots, ',');

    for (const auto &seg : segments) {
        if (seg.find('-') != std::string::npos) { // like "2-10"
            auto number_pair = StringUtil::split(seg, '-');
            if (number_pair.size() != 2) {
                return false;
            }
            int64_t n1 = 0, n2 = 0;
            if (!StringUtil::string2ll(number_pair[0].c_str(), number_pair[0].size(), &n1)) {
                return false;
            }
            if (!StringUtil::string2ll(number_pair[1].c_str(), number_pair[1].size(), &n2)) {
                return false;
            }
            if (n1 > n2) {
                return false;
            }
            if (!(0 <= n1 && n1 <= n2 && n2 < KeyHash::SLOTS_NUM)) {
                return false;
            }
            for (auto i = n1; i <= n2; i++) {
                if (tmp_bitset[i]) {
                    return false;
                }
                tmp_bitset.set(i, true);
            }
        } else { // like "1"
            int64_t n = 0;
            if (StringUtil::string2ll(seg.c_str(), seg.size(), &n) == false) {
                return false;
            }
            if (!(0 <= n && n < KeyHash::SLOTS_NUM)) {
                return false;
            }
            if (tmp_bitset[n]) {
                return false;
            }
            tmp_bitset.set(n, true);
        }
    }
    bitset_ = tmp_bitset;
    return true;
}

std::string SlotsBitset::toString() const {
    std::string str;
    int start = -1;
    for (int i = 0; i < KeyHash::SLOTS_NUM; i++) {
        // start
        if (bitset_[i] && start == -1) {
            start = i;
            if (!str.empty()) str.append(",");
            str.append(std::to_string(i));
        }
        // end
        if (!bitset_[i]) {
            if (start != -1 && start != i - 1) {
                str.append("-");
                str.append(std::to_string(i - 1));
            }
            start = -1;
        }
        if (bitset_[i] && i == KeyHash::SLOTS_NUM - 1 && start != i) {
            str.append("-");
            str.append(std::to_string(i));
        }
    }
    return str;
}

} // namespace tair::common
