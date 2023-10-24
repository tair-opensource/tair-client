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
#include "protocol/packet/resp/BulkStringPacket.hpp"

#include "absl/strings/numbers.h"

#include "common/Compiler.hpp"
#include "protocol/ProtocolOptions.hpp"

namespace tair::protocol {

using absl::SimpleAtoi;
using absl::string_view;

size_t BulkStringPacket::getEncodeSize() const {
    if (type_ == PacketType::TYPE_COMMON) {
        // $ and len and \r\n and data and \r\n
        return 1 + fmt::formatted_size("{}", bulk_str_.size()) + 2 + bulk_str_.size() + 2;
    } else {
        // $ -1 \r\n
        return 1 + 2 + 2;
    }
}

DState BulkStringPacket::encode(Buffer *buf, uint8_t packet_magic) {
    buf->appendInt8(packet_magic);
    if (type_ == PacketType::TYPE_COMMON) {
        buf->appendNumberToStr(bulk_str_.size());
        buf->appendCRLF();
        buf->append(bulk_str_);
    } else {
        buf->append("-1", 2);
    }
    buf->appendCRLF();
    return DState::SUCCESS;
}

DState BulkStringPacket::decode(Buffer *buf, uint8_t packet_magic) {
    if (unlikely(buf->empty())) {
        return DState::AGAIN;
    }
    if (decode_bulk_len_ == NOT_SET_SIZE) {
        if (unlikely(buf->peekInt8() != packet_magic)) {
            err_ = fmt::format("Protocol error: expected '{}', got '{}'", (char)packet_magic, (char)buf->peekInt8());
            return DState::ERROR;
        }
        const char *start = buf->data() + 1; // skip magic
        const char *newline = buf->findCRLF(start);
        if (unlikely(!newline)) {
            if (unlikely(buf->length() > ProtocolOptions::PROTO_RESP_INLINE_MAX_SIZE)) {
                err_ = "Protocol error: too big bulk count string";
                return DState::ERROR;
            }
            return DState::AGAIN;
        }
        if (unlikely(newline == start)) {
            err_ = "Protocol error: not found bulkstring len";
            return DState::ERROR;
        }
        if (unlikely(!SimpleAtoi(string_view(start, newline - start), &decode_bulk_len_))) {
            err_ = "Protocol error: invalid bulk length";
            return DState::ERROR;
        }
        if (unlikely(decode_bulk_len_ < -1 || decode_bulk_len_ > (int64_t)ProtocolOptions::proto_max_bulk_len)) {
            err_ = "Protocol error: invalid bulk length";
            return DState::ERROR;
        }
        buf->skip(newline - buf->data() + 2); // include last \r\n
        if (decode_bulk_len_ == -1) {
            type_ = PacketType::TYPE_NULL;
            return DState::SUCCESS;
        }
        if (decode_bulk_len_ >= (int64_t)ProtocolOptions::PROTO_RESP_MBULK_BIG_ARG) {
            if ((int64_t)buf->length() <= decode_bulk_len_ + 2) {
                buf->ensureWritableBytes(decode_bulk_len_ + 2 - buf->length());
            }
        }
    }
    if ((int64_t)buf->length() < decode_bulk_len_ + 2) {
        // Not enough data (+2 for trailing \r\n)
        return DState::AGAIN;
    }
    bulk_str_ = std::string(buf->data(), decode_bulk_len_);
    buf->skip(decode_bulk_len_ + 2); // include last \r\n
    decode_bulk_len_ = NOT_SET_SIZE;
    return DState::SUCCESS;
}

} // namespace tair::protocol
