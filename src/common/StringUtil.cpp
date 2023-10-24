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
#include "common/StringUtil.hpp"

#include <cerrno>
#include <climits>
#include <cmath>
#include <sys/time.h>
#include <unistd.h>

#include "common/Sha.hpp"

#include "absl/strings/numbers.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "fmt/format.h"

namespace tair::common {

using absl::SimpleAtoi;

static int is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int hex_digit_to_int(char c) {
    switch (c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a':
        case 'A': return 10;
        case 'b':
        case 'B': return 11;
        case 'c':
        case 'C': return 12;
        case 'd':
        case 'D': return 13;
        case 'e':
        case 'E': return 14;
        case 'f':
        case 'F': return 15;
        default: return 0;
    }
}

std::vector<std::string> StringUtil::split(const std::string &str, char delimiter) {
    return absl::StrSplit(str, delimiter);
}

bool StringUtil::splitArgs(const char *start, const char *end, std::vector<std::string> &args) {
    const char *p = start;
    while (true) {
        // skip blanks
        while (*p && p < end && std::isspace(*p)) {
            p++;
        }
        if (*p && p < end) {
            // get a token
            int inq = 0;  // set to 1 if we are in "quotes"
            int insq = 0; // set to 1 if we are in 'single quotes'
            int done = 0;
            std::string current;
            while (!done) {
                if (inq) {
                    if (*p == '\\' && *(p + 1) == 'x' && is_hex_digit(*(p + 2)) && is_hex_digit(*(p + 3))) {
                        unsigned char byte = (hex_digit_to_int(*(p + 2)) * 16) + hex_digit_to_int(*(p + 3));
                        current.push_back(byte);
                        p += 3;
                    } else if (*p == '\\' && *(p + 1)) {
                        char c;
                        p++;
                        switch (*p) {
                            case 'n': c = '\n'; break;
                            case 'r': c = '\r'; break;
                            case 't': c = '\t'; break;
                            case 'b': c = '\b'; break;
                            case 'a': c = '\a'; break;
                            default: c = *p; break;
                        }
                        current.push_back(c);
                    } else if (*p == '"') {
                        // closing quote must be followed by a space or nothing at all
                        if (*(p + 1) && !std::isspace(*(p + 1))) {
                            goto err;
                        }
                        done = 1;
                    } else if (!*p) {
                        // unterminated quotes
                        goto err;
                    } else {
                        current.push_back(*p);
                    }
                } else if (insq) {
                    if (*p == '\\' && *(p + 1) == '\'') {
                        p++;
                        current.push_back('\'');
                    } else if (*p == '\'') {
                        // closing quote must be followed by a space or nothing at all
                        if (*(p + 1) && !std::isspace(*(p + 1))) {
                            goto err;
                        }
                        done = 1;
                    } else if (!*p) {
                        // unterminated quotes
                        goto err;
                    } else {
                        current.push_back(*p);
                    }
                } else {
                    switch (*p) {
                        case ' ':
                        case '\n':
                        case '\r':
                        case '\t':
                        case '\0':
                            done = 1;
                            break;
                        case '"':
                            inq = 1;
                            break;
                        case '\'':
                            insq = 1;
                            break;
                        default:
                            current.push_back(*p);
                            break;
                    }
                }
                if (*p) {
                    p++;
                }
            }
            // add the token to the vector
            args.emplace_back(std::move(current));
        } else {
            return true;
        }
    }

err:
    args.clear();
    return false;
}

std::string StringUtil::join(const std::vector<std::string> &range, char separator) {
    std::string sep {separator};
    return absl::StrJoin(range, sep);
}

void StringUtil::toPrintableStr(std::string &out, const std::string_view &str) {
    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char ch = str[i];
        switch (ch) {
            case '"': out.append("\\\"", 2); break;
            case '\\': out.append("\\\\", 2); break;
            case '\n': out.append("\\n", 2); break;
            case '\r': out.append("\\r", 2); break;
            case '\t': out.append("\\t", 2); break;
            case '\a': out.append("\\a", 2); break;
            case '\b': out.append("\\b", 2); break;
            default:
                if (std::isprint(ch)) {
                    out.push_back(ch);
                } else {
                    char buf[5];
                    std::snprintf(buf, sizeof(buf), "\\x%02x", ch);
                    out.append(buf, 4);
                }
                break;
        }
    }
}

void StringUtil::toPrintableStr(std::string &out, const std::vector<std::string> &strs, bool haswarp) {
    if (haswarp) {
        out.push_back('[');
    }
    for (size_t i = 0; i < strs.size(); ++i) {
        if (haswarp) {
            out.push_back('\'');
        }
        StringUtil::toPrintableStr(out, strs[i]);
        if (haswarp) {
            out.push_back('\'');
        }
        if (i != strs.size() - 1) {
            if (haswarp) {
                out.push_back(',');
            }
            out.push_back(' ');
        }
    }
    if (haswarp) {
        out.push_back(']');
    }
}

void StringUtil::escapeJsonString(std::string &out, const std::string &str) {
    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char ch = str[i];
        switch (ch) {
            case '\\': out.append("\\\\", 2); break;
            case '"': out.append("\\\"", 2); break;
            case '\n': out.append("\\n", 2); break;
            case '\f': out.append("\\f", 2); break;
            case '\r': out.append("\\r", 2); break;
            case '\t': out.append("\\t", 2); break;
            case '\b': out.append("\\b", 2); break;
            default:
                if (std::isprint(ch)) {
                    out.push_back(ch);
                } else {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", ch);
                    out.append(buf, 6);
                }
                break;
        }
    }
}

bool matchLenImpl(const char *pattern, size_t plen, const char *str, size_t slen, bool nocase, int *skipLongerMatches) {
    while (plen && slen) {
        switch (pattern[0]) {
            case '*':
                while (pattern[1] == '*') {
                    pattern++;
                    plen--;
                }
                if (plen == 1) {
                    return true;
                }
                while (slen) {
                    if (matchLenImpl(pattern + 1, plen - 1, str, slen, nocase, skipLongerMatches)) {
                        return true;
                    }
                    if (*skipLongerMatches) {
                        return false;
                    }
                    str++;
                    slen--;
                }
                /* There was no match for the rest of the pattern starting
                 * from anywhere in the rest of the string. If there were
                 * any '*' earlier in the pattern, we can terminate the
                 * search early without trying to match them to longer
                 * substrings. This is because a longer match for the
                 * earlier part of the pattern would require the rest of the
                 * pattern to match starting later in the string, and we
                 * have just determined that there is no match for the rest
                 * of the pattern starting from anywhere in the current
                 * string. */
                *skipLongerMatches = 1;
                return false; /* no match */
                break;
            case '?':
                str++;
                slen--;
                break;
            case '[': {
                pattern++;
                plen--;
                bool no = pattern[0] == '^';
                if (no) {
                    pattern++;
                    plen--;
                }
                bool match = false;
                while (1) {
                    if (pattern[0] == '\\' && plen >= 2) {
                        pattern++;
                        plen--;
                        if (pattern[0] == str[0]) {
                            match = true;
                        }
                    } else if (pattern[0] == ']') {
                        break;
                    } else if (plen == 0) {
                        pattern--;
                        plen++;
                        break;
                    } else if (pattern[1] == '-' && plen >= 3) {
                        int start = pattern[0];
                        int end = pattern[2];
                        int c = str[0];
                        if (start > end) {
                            int t = start;
                            start = end;
                            end = t;
                        }
                        if (nocase) {
                            start = std::tolower(start);
                            end = std::tolower(end);
                            c = std::tolower(c);
                        }
                        pattern += 2;
                        plen -= 2;
                        if (c >= start && c <= end) {
                            match = true;
                        }
                    } else {
                        if (!nocase) {
                            if (pattern[0] == str[0])
                                match = true;
                        } else {
                            if (std::tolower(pattern[0]) == std::tolower(str[0])) {
                                match = true;
                            }
                        }
                    }
                    pattern++;
                    plen--;
                }
                if (no) {
                    match = !match;
                }
                if (!match) {
                    return false;
                }
                str++;
                slen--;
                break;
            }
            case '\\':
                if (plen >= 2) {
                    pattern++;
                    plen--;
                }
                /* fall through */
            default:
                if (!nocase) {
                    if (pattern[0] != str[0]) {
                        return false;
                    }
                } else {
                    if (std::tolower(pattern[0]) != std::tolower(str[0])) {
                        return false;
                    }
                }
                str++;
                slen--;
                break;
        }
        pattern++;
        plen--;
        if (slen == 0) {
            while (*pattern == '*') {
                pattern++;
                plen--;
            }
            break;
        }
    }
    if (plen == 0 && slen == 0) {
        return true;
    }

    return false;
}

bool StringUtil::matchLen(const char *pattern, size_t plen, const char *str, size_t slen, bool nocase) {
    int skipLongerMatches = 0;
    return matchLenImpl(pattern, plen, str, slen, nocase, &skipLongerMatches);
}

void StringUtil::getRandomBytes(unsigned char *p, size_t len) {
    static int seed_initialized = 0;
    static unsigned char seed[64]; // 512 bit internal block size
    static uint64_t counter = 0;   // The counter we hash with the seed

    if (!seed_initialized) {
        // Initialize a seed and use SHA1 in counter mode, where we hash the same seed with a progressive counter.
        // For the goals of this function we just need non-colliding strings, there are no cryptographic security needs.
        FILE *fp = ::fopen("/dev/urandom", "r");
        if (fp == NULL || ::fread(seed, sizeof(seed), 1, fp) != 1) {
            // Revert to a weaker seed, and in this case reseed again at every call
            for (unsigned int j = 0; j < sizeof(seed); j++) {
                struct timeval tv;
                ::gettimeofday(&tv, nullptr);
                pid_t pid = ::getpid();
                seed[j] = tv.tv_sec ^ tv.tv_usec ^ pid ^ (long)fp;
            }
        } else {
            seed_initialized = 1;
        }
        if (fp) ::fclose(fp);
    }

    while (len) {
        // This implements SHA256-HMAC
        unsigned char digest[SHA256_BLOCK_SIZE];
        unsigned char kxor[64];
        unsigned int copylen = len > SHA256_BLOCK_SIZE ? SHA256_BLOCK_SIZE : len;

        // IKEY: key xored with 0x36
        ::memcpy(kxor, seed, sizeof(kxor));
        for (unsigned int i = 0; i < sizeof(kxor); i++) {
            kxor[i] ^= 0x36;
        }

        // Obtain HASH(IKEY||MESSAGE)
        SHA256_CTX ctx;
        Sha256::init(&ctx);
        Sha256::update(&ctx, kxor, sizeof(kxor));
        Sha256::update(&ctx, (unsigned char *)&counter, sizeof(counter));
        Sha256::final(&ctx, digest);

        // OKEY: key xored with 0x5c
        ::memcpy(kxor, seed, sizeof(kxor));
        for (unsigned int i = 0; i < sizeof(kxor); i++) {
            kxor[i] ^= 0x5C;
        }

        // Obtain HASH(OKEY || HASH(IKEY||MESSAGE))
        Sha256::init(&ctx);
        Sha256::update(&ctx, kxor, sizeof(kxor));
        Sha256::update(&ctx, digest, SHA256_BLOCK_SIZE);
        Sha256::final(&ctx, digest);

        // Increment the counter for the next iteration
        counter++;

        ::memcpy(p, digest, copylen);
        len -= copylen;
        p += copylen;
    }
}

void StringUtil::getRandomHexChars(char *p, size_t len) {
    const char *charset = "0123456789abcdef";
    size_t j;
    getRandomBytes((unsigned char *)p, len);
    for (j = 0; j < len; j++) {
        p[j] = charset[p[j] & 0x0F];
    }
}

bool StringUtil::string2ld(const char *s, size_t slen, long double *dp) {
    char buf[MAX_LONG_DOUBLE_CHARS];
    if (slen == 0 || slen >= sizeof(buf)) {
        return false;
    }
    ::memcpy(buf, s, slen);
    buf[slen] = '\0';

    errno = 0;
    char *eptr;
    long double value = ::strtold(buf, &eptr);
    if (::isspace(buf[0]) || *eptr != '\0' || (size_t)(eptr - buf) != slen || (errno == ERANGE && (value == HUGE_VAL || value == -HUGE_VAL || value == 0)) || errno == EINVAL || std::isnan(value)) {
        return false;
    }
    if (dp) {
        *dp = value;
    }
    return true;
}

bool StringUtil::string2d(const char *s, size_t slen, double *dp) {
    char buf[MAX_LONG_DOUBLE_CHARS];
    if (slen == 0 || slen >= sizeof(buf)) {
        return false;
    }
    ::memcpy(buf, s, slen);
    buf[slen] = '\0';

    errno = 0;
    char *eptr;
    double value = ::strtod(buf, &eptr);
    if (::isspace(buf[0]) || *eptr != '\0' || (size_t)(eptr - buf) != slen || (errno == ERANGE && (value == HUGE_VAL || value == -HUGE_VAL || value == 0)) || errno == EINVAL || std::isnan(value)) {
        return false;
    }
    if (dp) {
        *dp = value;
    }
    return true;
}

bool StringUtil::string2IntArr(const std::string &str, std::vector<int> &arr) {
    if (str.empty()) {
        return false;
    }
    auto args = split(str, ',');
    // prev_num used for check
    int prev_num = -1;
    for (auto &arg : args) {
        auto vals = split(arg, '-');
        if (vals.size() == 1) {
            int num;
            if (!SimpleAtoi(vals[0], &num)) {
                return false;
            }
            arr.emplace_back(num);
            prev_num = num;
        } else if (vals.size() == 2) {
            int start, end;
            if (!SimpleAtoi(vals[0], &start) || !SimpleAtoi(vals[1], &end)) {
                return false;
            }
            if (start > end || start <= prev_num) {
                return false;
            }
            for (int i = start; i <= end; ++i) {
                arr.emplace_back(i);
            }
            prev_num = end;
        } else {
            return false;
        }
    }
    return true;
}

void StringUtil::stringMapChars(std::string &str, const char *from, const char *to, size_t setlen) {
    for (size_t i = 0; i < str.size(); ++i) {
        for (size_t j = 0; j < setlen; ++j) {
            if (str[i] == from[j]) {
                str[i] = to[j];
                break;
            }
        }
    }
}

int StringUtil::ld2string(char *buf, size_t len, long double value, bool humanfriendly) {
    int l;
    if (std::isinf(value)) {
        /* Libc in odd systems (Hi Solaris!) will format infinite in a
         * different way, so better to handle it in an explicit way. */
        if (len < 5) {
            return false; /* No room. 5 is "-inf\0" */
        }
        if (value > 0) {
            ::memcpy(buf, "inf", 3);
            l = 3;
        } else {
            ::memcpy(buf, "-inf", 4);
            l = 4;
        }
    } else if (humanfriendly) {
        /* We use 17 digits precision since with 128 bit floats that precision
         * after rounding is able to represent most small decimal numbers in a
         * way that is "non surprising" for the user (that is, most small
         * decimal numbers will be represented in a way that when converted
         * back into a string are exactly the same as what the user typed.) */
        l = ::snprintf(buf, len, "%.17Lf", value);
        if (l + 1 > (int)len) {
            return false; /* No room. */
        }
        /* Now remove trailing zeroes after the '.' */
        if (::strchr(buf, '.')) {
            char *p = buf + l - 1;
            while (*p == '0') {
                p--;
                l--;
            }
            if (*p == '.') l--;
        }
        if (l == 2 && buf[0] == '-' && buf[1] == '0') {
            buf[0] = '0';
            l = 1;
        }
    } else {
        l = ::snprintf(buf, len, "%.17Lg", value);
        if (l + 1 > (int)len) {
            return false; /* No room. */
        }
    }
    buf[l] = '\0';
    return l;
}

int StringUtil::string2ull(const char *s, size_t slen, uint64_t *value) {
    const char *p = s;
    size_t plen = 0;
    unsigned long v;

    /* A zero length string is not a valid number. */
    if (plen == slen)
        return 0;

    /* Special case: first and only digit is 0. */
    if (slen == 1 && p[0] == '0') {
        if (value != NULL) *value = 0;
        return 1;
    }

    /* First digit should be 1-9, otherwise the string should just be 0. */
    if (p[0] >= '1' && p[0] <= '9') {
        v = p[0] - '0';
        p++;
        plen++;
    } else {
        return 0;
    }

    /* Parse all the other digits, checking for overflow at every step. */
    while (plen < slen && p[0] >= '0' && p[0] <= '9') {
        if (v > (ULLONG_MAX / 10)) /* Overflow. */
            return 0;
        v *= 10;

        if (v > (ULLONG_MAX - (p[0] - '0'))) /* Overflow. */
            return 0;
        v += p[0] - '0';

        p++;
        plen++;
    }

    /* Return if not all bytes were used. */
    if (plen < slen)
        return 0;

    if (value != NULL) *value = v;
    return 1;
}

int StringUtil::string2ll(const char *s, size_t slen, int64_t *value) {
    const char *p = s;
    size_t plen = 0;
    int negative = 0;
    unsigned long v;

    /* A zero length string is not a valid number. */
    if (plen == slen)
        return 0;

    /* Special case: first and only digit is 0. */
    if (slen == 1 && p[0] == '0') {
        if (value != NULL) *value = 0;
        return 1;
    }

    /* Handle negative numbers: just set a flag and continue like if it
     * was a positive number. Later convert into negative. */
    if (p[0] == '-') {
        negative = 1;
        p++;
        plen++;

        /* Abort on only a negative sign. */
        if (plen == slen)
            return 0;
    }

    /* First digit should be 1-9, otherwise the string should just be 0. */
    if (p[0] >= '1' && p[0] <= '9') {
        v = p[0] - '0';
        p++;
        plen++;
    } else {
        return 0;
    }

    /* Parse all the other digits, checking for overflow at every step. */
    while (plen < slen && p[0] >= '0' && p[0] <= '9') {
        if (v > (ULLONG_MAX / 10)) /* Overflow. */
            return 0;
        v *= 10;

        if (v > (ULLONG_MAX - (p[0] - '0'))) /* Overflow. */
            return 0;
        v += p[0] - '0';

        p++;
        plen++;
    }

    /* Return if not all bytes were used. */
    if (plen < slen)
        return 0;

    /* Convert to negative if needed, and do the final overflow check when
     * converting from unsigned long long to long long. */
    if (negative) {
        if (v > ((unsigned long)(-(LLONG_MIN + 1)) + 1)) /* Overflow. */
            return 0;
        if (value != NULL) *value = -v;
    } else {
        if (v > LLONG_MAX) /* Overflow. */
            return 0;
        if (value != NULL) *value = v;
    }
    return 1;
}

int64_t StringUtil::memtoll(const char *p, int *err) {
    const char *u;
    char buf[128];
    long mul; /* unit multiplier */
    int64_t val;
    unsigned int digits;
    if (err) *err = 0;

    /* Search the first non digit character. */
    u = p;
    if (*u == '-') u++;
    while (*u && isdigit(*u)) u++;
    if (*u == '\0' || equalsNoCase(u, "b")) {
        mul = 1;
    } else if (equalsNoCase(u, "k")) {
        mul = 1000;
    } else if (equalsNoCase(u, "kb")) {
        mul = 1024;
    } else if (equalsNoCase(u, "m")) {
        mul = 1000 * 1000;
    } else if (equalsNoCase(u, "mb")) {
        mul = 1024 * 1024;
    } else if (equalsNoCase(u, "g")) {
        mul = 1000L * 1000 * 1000;
    } else if (equalsNoCase(u, "gb")) {
        mul = 1024L * 1024 * 1024;
    } else {
        if (err) *err = 1;
        return 0;
    }
    /* Copy the digits into a buffer, we'll use strtoll() to convert
     * the digit (without the unit) into a number. */
    digits = u - p;
    if (digits >= sizeof(buf)) {
        if (err) *err = 1;
        return 0;
    }
    ::memcpy(buf, p, digits);
    buf[digits] = '\0';

    char *endptr;
    errno = 0;
    val = ::strtoll(buf, &endptr, 10);
    if ((val == 0 && errno == EINVAL) || *endptr != '\0') {
        if (err) *err = 1;
        return 0;
    }
    return val * mul;
}

std::string StringUtil::lltomem(long long bytes) {
    const int64_t gb = 1024 * 1024 * 1024;
    const int64_t mb = 1024 * 1024;
    const int64_t kb = 1024;
    if (bytes && (bytes % gb) == 0) {
        return fmt::format("{}gb", bytes / gb);
    } else if (bytes && (bytes % mb) == 0) {
        return fmt::format("{}mb", bytes / mb);
    } else if (bytes && (bytes % kb) == 0) {
        return fmt::format("{}kb", bytes / kb);
    } else {
        return fmt::format("{}", bytes);
    }
}

std::string StringUtil::bytesToHuman(uint64_t n) {
    if (n < 1024) {
        return fmt::format("{}B", n);
    } else if (n < (1024 * 1024)) {
        return fmt::format("{:.2f}K", (double)n / (1024));
    } else if (n < (1024LL * 1024 * 1024)) {
        return fmt::format("{:.2f}M", (double)n / (1024 * 1024));
    } else if (n < (1024LL * 1024 * 1024 * 1024)) {
        return fmt::format("{:.2f}G", (double)n / (1024LL * 1024 * 1024));
    } else if (n < (1024LL * 1024 * 1024 * 1024 * 1024)) {
        return fmt::format("{:.2f}T", (double)n / (1024LL * 1024 * 1024 * 1024));
    } else if (n < (1024LL * 1024 * 1024 * 1024 * 1024 * 1024)) {
        return fmt::format("{:.2f}P", (double)n / (1024LL * 1024 * 1024 * 1024 * 1024));
    }
    // Let's hope we never need this
    return fmt::format("{}B", n);
}

} // namespace tair::common
