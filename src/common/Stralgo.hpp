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

#include <string>
#include <vector>

namespace tair::common {

class Stralgo {
public:
    struct LcsMatchPos {
        template <typename T>
        LcsMatchPos(T &&pa, T &&pb, uint32_t len)
            : pos_a(std::forward<T>(pa)),
              pos_b(std::forward<T>(pb)),
              match_len(len) {}

        std::pair<uint32_t, uint32_t> pos_a;
        std::pair<uint32_t, uint32_t> pos_b;
        uint32_t match_len;
    };

    static void lcs(std::string_view a, std::string_view b, bool getidx, bool getlen, uint32_t minmatchlen,
                    std::string &matchstr, uint32_t &len, std::vector<LcsMatchPos> &matches);
};

} // namespace tair::common
