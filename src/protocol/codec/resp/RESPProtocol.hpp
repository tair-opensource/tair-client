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

#include <cstdint>

#define SIMPLE_STRING_PACKET_MAGIC   '+'
#define ERROR_PACKET_MAGIC           '-'
#define INTEGER_PACKET_MAGIC         ':'
#define BULK_STRING_PACKET_MAGIC     '$'
#define ARRAY_PACKET_MAGIC           '*'
#define NULL_PACKET_MAGIC            '_'
#define DOUBLE_PACKET_MAGIC          ','
#define BOOLEAN_PACKET_MAGIC         '#'
#define BLOB_ERROR_PACKET_MAGIC      '!'
#define VERBATIM_STRING_PACKET_MAGIC '='
#define BIG_NUMBER_PACKET_MAGIC      '('
#define MAP_PACKET_MAGIC             '%'
#define SET_PACKET_MAGIC             '~'
#define ATTRIBUTE_PACKET_MAGIC       '|'
#define PUSH_PACKET_MAGIC            '>'

#define STREAM_START_FLAG "$EOF:" // for Stream Mode start

namespace tair::protocol {

// the size of array or bulkstring allow 0 or -1
constexpr const int64_t NOT_SET_SIZE = -2;

} // namespace tair::protocol
