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
#include "protocol/packet/resp/MapPacket.hpp"

#include "absl/strings/numbers.h"

#include "common/Compiler.hpp"
#include "protocol/ProtocolOptions.hpp"
#include "protocol/packet/resp/RESPPacketFactory.hpp"

namespace tair::protocol {

using absl::SimpleAtoi;
using absl::string_view;

size_t MapPacket::getEncode2Size() const {
    // * and number + \r\n
    size_t size = 1 + fmt::formatted_size("{}", packet_array_.size() * 2) + 2;
    for (const auto &pair : packet_array_) {
        size += pair.first->getRESP2EncodeSize();
        size += pair.second->getRESP2EncodeSize();
    }
    return size;
}

DState MapPacket::encode2(Buffer *buf, uint8_t packet_magic) {
    buf->appendInt8(packet_magic);
    buf->appendNumberToStr(packet_array_.size() * 2);
    buf->appendCRLF();
    for (const auto &pair : packet_array_) {
        pair.first->encodeRESP2(buf);
        pair.second->encodeRESP2(buf);
    }
    return DState::SUCCESS;
}

DState MapPacket::decode2(Buffer *buf UNUSED, uint8_t packet_magic UNUSED) {
    // not support from RESP2Codec
    return DState::ERROR;
}

size_t MapPacket::getEncode3Size() const {
    // % and number + \r\n
    size_t size = 1 + fmt::formatted_size("{}", packet_array_.size()) + 2;
    for (const auto &pair : packet_array_) {
        size += pair.first->getRESP3EncodeSize();
        size += pair.second->getRESP3EncodeSize();
    }
    return size;
}

DState MapPacket::encode3(Buffer *buf, uint8_t packet_magic) {
    buf->appendInt8(packet_magic);
    buf->appendNumberToStr(packet_array_.size());
    buf->appendCRLF();
    for (const auto &pair : packet_array_) {
        pair.first->encodeRESP3(buf);
        pair.second->encodeRESP3(buf);
    }
    return DState::SUCCESS;
}

DState MapPacket::decode3(Buffer *buf, uint8_t packet_magic) {
    if (unlikely(buf->empty())) {
        return DState::AGAIN;
    }
    if (decode_map_size_ == NOT_SET_SIZE) {
        runtimeAssert(decode_packet_count_ == NOT_SET_SIZE);
        if (unlikely(buf->peekInt8() != packet_magic)) {
            err_ = fmt::format("Protocol error: expected '{}', got '{}'", (char)packet_magic, (char)buf->peekInt8());
            return DState::ERROR;
        }
        const char *start = buf->data() + 1; // skip magic
        const char *newline = buf->findCRLF(start);
        if (!newline) {
            if (unlikely(buf->length() > ProtocolOptions::PROTO_RESP_INLINE_MAX_SIZE)) {
                err_ = "Protocol error: too big count string";
                return DState::ERROR;
            }
            return DState::AGAIN;
        }
        if (unlikely(newline == start)) {
            err_ = "Protocol error: not found array size";
            return DState::ERROR;
        }
        if (unlikely(!SimpleAtoi(string_view(start, newline - start), &decode_map_size_) || decode_map_size_ < 0)) {
            err_ = "Protocol error: integer format error";
            return DState::ERROR;
        }
        decode_packet_count_ = decode_map_size_ * 2;
        buf->skip(newline - buf->data() + 2); // include last \r\n
    }
    while (decode_packet_count_ > 0) {
        if (buf->empty()) {
            return DState::AGAIN;
        }
        if (!curr_parse_packet_) {
            char ch = buf->peekInt8();
            curr_parse_packet_ = RESPPacketFactory::createPacket(ch);
            if (unlikely(!curr_parse_packet_)) {
                err_ = fmt::format("Protocol error: unknown packet type: '{}'", ch);
                return DState::ERROR;
            }
        }
        auto dstate = curr_parse_packet_->decodeRESP3(buf);
        if (dstate == DState::SUCCESS) {
            if (!curr_pair_key_) {
                curr_pair_key_ = std::move(curr_parse_packet_);
                curr_parse_packet_.release();
            } else {
                addPacketPair(curr_pair_key_.get(), curr_parse_packet_.get());
                curr_pair_key_.release();
                curr_parse_packet_.release();
            }
            decode_packet_count_--;
        } else if (dstate == DState::ERROR) {
            err_ = curr_parse_packet_->getDecodeErr();
            curr_parse_packet_.reset();
            curr_pair_key_.reset();
            return dstate;
        } else {
            return dstate;
        }
    }
    return DState::SUCCESS;
}

} // namespace tair::protocol
