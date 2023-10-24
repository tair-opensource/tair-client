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
#include "common/TokenBucket.hpp"

#include "common/Assert.hpp"
#include "common/ClockTime.hpp"

namespace tair::common {

TokenBucket::TokenBucket(int64_t init_tokens, int64_t tokens_per_second, uint32_t strict_percent) {
    runtimeAssert(tokens_per_second > 0);
    tokens_ = init_tokens;
    max_tokens_ = tokens_;
    tokens_per_sec_ = tokens_per_second;
    last_fill_time_ms_ = ClockTime::intervalMs();
    strict_limit_factor_ = strict_percent / 100.0;
    if (strict_limit_factor_ < 1.0) {
        strict_limit_factor_ = 1.0;
    }
    strict_limit_tokens_ = max_tokens_ * strict_limit_factor_;
    strict_limit_max_tokens_ = strict_limit_tokens_;
}

bool TokenBucket::consume(int64_t tokens, int64_t &estimate_ms) {
    refillTokens();
    strict_limit_tokens_ -= tokens;
    if (tokens_ >= tokens) {
        tokens_ -= tokens;
        return true;
    }
    int64_t tokens_needed = tokens - tokens_;
    estimate_ms = tokens_needed * 1000 / tokens_per_sec_;
    tokens_ = 0;
    return false;
}

bool TokenBucket::isStrictLimit() {
    refillTokens();
    return strict_limit_tokens_ <= 0;
}

void TokenBucket::refillTokens() {
    int64_t now = ClockTime::intervalMs();
    if (now > last_fill_time_ms_) {
        int64_t new_tokens = (now - last_fill_time_ms_) * tokens_per_sec_ / 1000;
        tokens_ += new_tokens;
        if (tokens_ > max_tokens_) {
            tokens_ = max_tokens_;
        }
        strict_limit_tokens_ += new_tokens * strict_limit_factor_;
        if (strict_limit_tokens_ > strict_limit_max_tokens_) {
            strict_limit_tokens_ = strict_limit_max_tokens_;
        }
        last_fill_time_ms_ = now;
    }
}

} // namespace tair::common
