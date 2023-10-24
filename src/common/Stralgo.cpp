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
#include "common/Stralgo.hpp"

namespace tair::common {

// Compute the LCS using the vanilla dynamic programming technique of building a table of LCS(x,y) substrings.
void Stralgo::lcs(std::string_view a, std::string_view b, bool getidx, bool getlen, uint32_t minmatchlen,
                  std::string &matchstr, uint32_t &len, std::vector<LcsMatchPos> &matches) {
    // Detect string truncation or later overflows.
    if (a.size() >= UINT32_MAX - 1 || b.size() >= UINT32_MAX - 1) {
        return;
    }

    uint32_t alen = a.size(), blen = b.size();

    // Setup an uint32_t array to store at LCS[i,j] the length of the LCS A0..i-1, B0..j-1.
    // Note that we have a linear array here, so we index it as LCS[j+(blen+1)*j]

#define LCS(A, B) lcs[(B) + ((A) * (blen + 1))]

    // Try to allocate the LCS table, and abort on overflow or insufficient memory.
    unsigned long long lcssize = (unsigned long long)(alen + 1) * (blen + 1);
    unsigned long long lcsalloc = lcssize * sizeof(uint32_t);
    uint32_t *lcs = nullptr;
    if (lcsalloc < SIZE_MAX && lcsalloc / lcssize == sizeof(uint32_t)) {
        lcs = new uint32_t[lcsalloc];
    } else {
        return;
    }
    // Start building the LCS table.
    for (uint32_t i = 0; i <= alen; i++) {
        for (uint32_t j = 0; j <= blen; j++) {
            if (i == 0 || j == 0) {
                // If one substring has length of zero, the LCS length is zero.
                LCS(i, j) = 0;
            } else if (a[i - 1] == b[j - 1]) {
                // The len LCS (and the LCS itself) of two sequences with the same final character,
                // is the LCS of the two sequences without the last char plus that last char.
                LCS(i, j) = LCS(i - 1, j - 1) + 1;
            } else {
                // If the last character is different, take the longest between the LCS of the first string
                // and the second minus the last char, and the reverse.
                uint32_t lcs1 = LCS(i - 1, j);
                uint32_t lcs2 = LCS(i, j - 1);
                LCS(i, j) = lcs1 > lcs2 ? lcs1 : lcs2;
            }
        }
    }

    // Store the actual LCS string in "result" if needed.
    // We create it backward, but the length is already known, we store it into idx.
    uint32_t idx = LCS(alen, blen);
    uint32_t arange_start = alen; // alen signals that values are not set.
    uint32_t arange_end = 0, brange_start = 0, brange_end = 0;

    // Do we need to compute the actual LCS string? Allocate it in that case.
    bool computelcs = getidx || !getlen;
    if (computelcs) {
        matchstr.resize(idx);
    }

    uint32_t i = alen, j = blen;
    while (computelcs && i > 0 && j > 0) {
        int emit_range = 0;
        if (a[i - 1] == b[j - 1]) {
            // If there is a match, store the character and reduce the indexes to look for a new match.
            matchstr[idx - 1] = a[i - 1];

            // Track the current range.
            if (arange_start == alen) {
                arange_start = i - 1;
                arange_end = i - 1;
                brange_start = j - 1;
                brange_end = j - 1;
            } else {
                // Let's see if we can extend the range backward since it is contiguous.
                if (arange_start == i && brange_start == j) {
                    arange_start--;
                    brange_start--;
                } else {
                    emit_range = 1;
                }
            }
            // Emit the range if we matched with the first byte of one of the two strings. We'll exit the loop ASAP.
            if (arange_start == 0 || brange_start == 0) {
                emit_range = 1;
            }
            idx--;
            i--;
            j--;
        } else {
            // Otherwise reduce i and j depending on the largest LCS between, to understand what direction we need to go.
            uint32_t lcs1 = LCS(i - 1, j);
            uint32_t lcs2 = LCS(i, j - 1);
            if (lcs1 > lcs2) {
                i--;
            } else {
                j--;
            }
            if (arange_start != alen) {
                emit_range = 1;
            }
        }

        // Emit the current range if needed.
        uint32_t match_len = arange_end - arange_start + 1;
        if (emit_range) {
            if (minmatchlen == 0 || match_len >= minmatchlen) {
                if (getidx) {
                    LcsMatchPos pos(std::make_pair(arange_start, arange_end),
                                    std::make_pair(brange_start, brange_end),
                                    match_len);
                    matches.emplace_back(pos);
                }
            }
            arange_start = alen; // Restart at the next match.
        }
    }
    len = LCS(alen, blen);

    delete[] lcs;
}

} // namespace tair::common
