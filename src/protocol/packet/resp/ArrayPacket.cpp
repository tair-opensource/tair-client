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
#include "protocol/packet/resp/ArrayPacket.hpp"

#include "absl/strings/numbers.h"

#include "common/Compiler.hpp"
#include "protocol/ProtocolOptions.hpp"
#include "protocol/packet/resp/MapPacket.hpp"
#include "protocol/packet/resp/RESPPacketFactory.hpp"

namespace tair::protocol {

using absl::SimpleAtoi;
using absl::string_view;

SetPacket *ArrayPacket::startNewSet() {
    SetPacket *set = new SetPacket();
    addPacket(set);
    return set;
}

MapPacket *ArrayPacket::startNewMap() {
    MapPacket *map = new MapPacket();
    addPacket(map);
    return map;
}

size_t ArrayPacket::getEncodeSize(CodecType type) const {
    if (type_ == PacketType::TYPE_COMMON) {
        // * and number + \r\n
        size_t size = 1 + fmt::formatted_size("{}", packet_array_.size()) + 2;
        for (const auto &packet : packet_array_) {
            if (type == CodecType::RESP2) {
                size += packet->getRESP2EncodeSize();
            } else {
                size += packet->getRESP3EncodeSize();
            }
        }
        return size;
    } else {
        // * -1 \r\n
        return 1 + 2 + 2;
    }
}

DState ArrayPacket::encode(Buffer *buf, uint8_t packet_magic, CodecType type) {
    buf->appendInt8(packet_magic);
    if (type_ == PacketType::TYPE_COMMON) {
        buf->appendNumberToStr(packet_array_.size());
        buf->appendCRLF();
        for (auto packet : packet_array_) {
            if (type == CodecType::RESP2) {
                packet->encodeRESP2(buf);
            } else {
                packet->encodeRESP3(buf);
            }
        }
    } else {
        buf->append("-1", 2);
        buf->appendCRLF();
    }
    return DState::SUCCESS;
}

DState ArrayPacket::decode(Buffer *buf, uint8_t packet_magic, CodecType type) {
    if (unlikely(buf->empty())) {
        return DState::AGAIN;
    }
    if (decode_array_size_ == NOT_SET_SIZE) {
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
        if (unlikely(!SimpleAtoi(string_view(start, newline - start), &decode_array_size_))) {
            err_ = "Protocol error: integer format error";
            return DState::ERROR;
        }
        buf->skip(newline - buf->data() + 2); // include last \r\n
        if (unlikely(decode_size_limit_ > 0 && decode_array_size_ > decode_size_limit_)) {
            err_ = "Protocol error: invalid multibulk length";
            return DState::ERROR;
        }
        if (decode_array_size_ < 0) {
            type_ = PacketType::TYPE_NULL;
        }
    }
    while (decode_array_size_ > 0) {
        if (buf->empty()) {
            return DState::AGAIN;
        }
        if (!curr_parse_packet_) {
            char ch = buf->peekInt8();
            curr_parse_packet_ = RESPPacketFactory::createPacket(ch);
            if (!curr_parse_packet_) {
                auto out = StringUtil::toPrintableStr(std::string(1, ch));
                err_ = fmt::format("Protocol error: unknown packet type: '{}'", out);
                return DState::ERROR;
            }
        }
        DState dstate;
        if (type == CodecType::RESP2) {
            dstate = curr_parse_packet_->decodeRESP2(buf);
        } else {
            dstate = curr_parse_packet_->decodeRESP3(buf);
        }
        if (dstate == DState::SUCCESS) {
            addPacket(curr_parse_packet_.get());
            curr_parse_packet_.release();
            decode_array_size_--;
        } else if (dstate == DState::ERROR) {
            err_ = curr_parse_packet_->getDecodeErr();
            curr_parse_packet_.reset();
            return dstate;
        } else {
            return dstate;
        }
    }
    return DState::SUCCESS;
}

size_t ArrayPacket::getEncode2Size() const {
    return getEncodeSize(CodecType::RESP2);
}

DState ArrayPacket::encode2(Buffer *buf, uint8_t packet_magic) {
    return encode(buf, packet_magic, CodecType::RESP2);
}

DState ArrayPacket::decode2(Buffer *buf, uint8_t packet_magic) {
    return decode(buf, packet_magic, CodecType::RESP2);
}

size_t ArrayPacket::getEncode3Size() const {
    return getEncodeSize(CodecType::RESP3);
}

DState ArrayPacket::encode3(Buffer *buf, uint8_t packet_magic) {
    return encode(buf, packet_magic, CodecType::RESP3);
}

DState ArrayPacket::decode3(Buffer *buf, uint8_t packet_magic) {
    return decode(buf, packet_magic, CodecType::RESP3);
}

} // namespace tair::protocol
