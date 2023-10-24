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

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

// The maximum number of characters needed to represent a long double as a string (long double has a huge range).
// This should be the size of the buffer given to ld2string
#define MAX_LONG_DOUBLE_CHARS (5 * 1024)

namespace tair::common {

class StringUtil {
public:
    static inline void toUpper(std::string &str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }

    static inline void toLower(std::string &str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    static inline bool equalsNoCase(const std::string &x, const std::string &y) {
        return ::strcasecmp(x.c_str(), y.c_str()) == 0;
    }

    static inline bool equalsNoCase(const char *x, const std::string &y) {
        return ::strcasecmp(x, y.c_str()) == 0;
    }

    static inline bool equalsNoCase(const std::string &x, const char *y) {
        return ::strcasecmp(x.c_str(), y) == 0;
    }

    static inline bool equalsNoCase(const std::string *x, const std::string *y) {
        return ::strcasecmp(x->c_str(), y->c_str()) == 0;
    }

    static inline bool equalsNoCase(const std::string *x, const char *y) {
        return ::strcasecmp(x->c_str(), y) == 0;
    }

    static inline bool equalsNoCase(const char *x, const std::string *y) {
        return ::strcasecmp(x, y->c_str()) == 0;
    }

    static inline bool equalsNoCase(const char *x, const char *y) {
        return ::strcasecmp(x, y) == 0;
    }

    static inline bool equalsCase(const char *x, size_t x_len, const char *y, size_t y_len) {
        if (x_len != y_len) {
            return false;
        }
        return ::strncmp(x, y, x_len) == 0;
    }

    static inline bool equalsCase(const char *x, const char *y, size_t y_len) {
        return equalsCase(x, strlen(x), y, y_len);
    }

    static inline bool equalsCase(const std::string &x, const char *y, size_t len) {
        return equalsCase(x.data(), x.size(), y, len);
    }

    static inline bool startWith(const std::string &x, const char *y) {
        return ::strncmp(x.c_str(), y, strlen(y)) == 0;
    }

    static inline bool startWith(const std::string &x, const std::string &y) {
        return ::strncmp(x.c_str(), y.c_str(), y.size()) == 0;
    }

    static inline bool startWithNoCase(const std::string &x, const char *y) {
        return ::strncasecmp(x.c_str(), y, strlen(y)) == 0;
    }

    static inline bool startWithNoCase(const std::string &x, const std::string &y) {
        return ::strncasecmp(x.c_str(), y.c_str(), y.size()) == 0;
    }

    static std::vector<std::string> split(const std::string &str, char delimiter);

    // Split a line into arguments, where every argument can be in the following programming-language REPL-alike form:
    // foo bar "newline are supported\n" and "\xff\x00otherstuff"
    static bool splitArgs(const char *start, const char *end, std::vector<std::string> &args);

    static inline bool splitArgs(const std::string &line, std::vector<std::string> &args) {
        return splitArgs(line.c_str(), line.c_str() + line.size(), args);
    }

    static std::string join(const std::vector<std::string> &range, char delimiter);

    static inline void trim(std::string &str, const std::string delims = " \f\r\n\t\v") {
        str.erase(0, str.find_first_not_of(delims));
        str.erase(str.find_last_not_of(delims) + 1);
    }

    static void toPrintableStr(std::string &out, const std::string_view &str);
    static std::string toPrintableStr(const std::string_view &str) {
        std::string out;
        toPrintableStr(out, str);
        return out;
    }
    static void toPrintableStr(std::string &out, const std::vector<std::string> &strs, bool haswarp = true);
    static std::string toPrintableStr(const std::vector<std::string> &strs, bool haswarp = true) {
        std::string out;
        toPrintableStr(out, strs, haswarp);
        return out;
    }

    // Escape a Unicode string for JSON output (--json), following RFC 7159:
    // https://datatracker.ietf.org/doc/html/rfc7159#section-7
    static void escapeJsonString(std::string &out, const std::string &str);
    static std::string escapeJsonString(const std::string &str) {
        std::string out;
        escapeJsonString(out, str);
        return out;
    }

    static inline bool existsPattern(const std::string &pattern) {
        return pattern.find_first_of("*?[\\") == std::string::npos;
    }

    // modify from redis util.c
    static bool matchLen(const char *pattern, size_t plen, const char *str, size_t slen, bool nocase);

    static inline bool match(const std::string &pattern, const char *str, size_t slen) {
        return matchLen(pattern.c_str(), pattern.size(), str, slen, false);
    }

    static inline bool match(const std::string &pattern, const std::string &str) {
        return matchLen(pattern.c_str(), pattern.size(), str.c_str(), str.size(), false);
    }

    static inline bool matchNoCase(const std::string &pattern, const std::string &str) {
        return matchLen(pattern.c_str(), pattern.size(), str.c_str(), str.size(), true);
    }

    static void getRandomBytes(unsigned char *p, size_t len);
    static void getRandomHexChars(char *p, size_t len);

    // Convert a string into a double. Returns 1 if the string could be parsed into a (non-overflowing) double, 0 otherwise.
    // The value will be set to the parsed value when appropriate.
    // Note that this function demands that the string strictly represents a double: no spaces or other characters before
    // or after the string representing the number are accepted.
    static bool string2ld(const char *s, size_t slen, long double *dp);

    static inline bool string2ld(const std::string &str, long double *dp) {
        return string2ld(str.data(), str.size(), dp);
    }

    static bool string2d(const char *s, size_t slen, double *dp);

    static inline bool string2d(const std::string &str, double *dp) {
        return string2d(str.data(), str.size(), dp);
    }

    // Convert a string into an int array, string format is like: "0, [3-8]".
    // Return 1 if the string could be parsed into an int array.
    static bool string2IntArr(const std::string &str, std::vector<int> &arr);

    static void stringMapChars(std::string &str, const char *from, const char *to, size_t setlen);

    // Convert a long double into a string. If humanfriendly is non-zero it does not use exponential format
    // and trims trailing zeroes at the end, however this results in loss of precision.
    // Otherwise exp format is used and the output of snprintf() is not modified.
    // The function returns the length of the string or zero if there was not enough buffer room to store it.
    static int ld2string(char *buf, size_t len, long double value, bool humanfriendly);

    // Convert a string into a long long. Returns 1 if the string could be parsed into a (non-overflowing) long long, 0 otherwise.
    // The value will be set to the parsed value when appropriate.
    // Note that this function demands that the string strictly represents a long long:
    // no spaces or other characters before or after the string representing the number are accepted,
    // nor zeroes at the start if not for the string "0" representing the zero number.
    //
    // Because of its strictness, it is safe to use this function to check if you can convert a string
    // into a long long, and obtain back the string from the number without any loss in the string representation.
    static int string2ll(const char *s, size_t slen, int64_t *value);
    static int string2ull(const char *s, size_t slen, uint64_t *value);

    static int string2ll(const std::string &str, int64_t *value) {
        return string2ll(str.data(), str.size(), value);
    }

    // Convert a string representing an amount of memory into the number of bytes, so for instance memtoll("1Gb")
    // will return 1073741824 that is (1024*1024*1024).
    //
    // On parsing error, if *err is not NULL, it's set to 1, otherwise it's set to 0.
    // On error the function return value is 0, regardless of the fact 'err' is NULL or not.
    static int64_t memtoll(const char *p, int *err);
    static int64_t memtoll(const std::string &str, int *err) {
        return memtoll(str.c_str(), err);
    }

    static std::string lltomem(long long bytes);

    template <typename T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
    static std::string bytesToHuman(T n) {
        return bytesToHuman(static_cast<uint64_t>(n));
    }

    // Convert an amount of bytes into a human readable string in the form of 100B, 2G, 100M, 4K, and so forth.
    static std::string bytesToHuman(uint64_t n);
};

} // namespace tair::common
