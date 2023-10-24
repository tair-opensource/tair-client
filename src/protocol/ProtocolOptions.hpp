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

#include <atomic>
#include <cstddef>
#include <limits>

namespace tair::protocol {

class ProtocolOptions {
public:
    // IMMUTABLE_CONFIG
    const constexpr static size_t PROTO_RESP_INLINE_MAX_SIZE = 1024 * 64;
    const constexpr static size_t PROTO_RESP_MBULK_BIG_ARG = 1024 * 32;
    const constexpr static size_t PROTO_RESP_DECODE_REQUEST_SIZE_LIMIT = std::numeric_limits<int32_t>::max();
    const constexpr static size_t PROTO_MEMCACHED_KEY_MAX_LENGTH = 250;

    // MODIFIABLE_CONFIG
    static std::atomic<size_t> proto_max_bulk_len;
    static std::atomic<size_t> memcached_max_item_size;
};

} // namespace tair::protocol
