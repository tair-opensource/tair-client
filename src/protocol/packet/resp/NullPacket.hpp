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

#include "common/Compiler.hpp"
#include "protocol/packet/Packet.hpp"

namespace tair::protocol {

enum class NullType {
    null_bulk = 0,
    null_multi_bulk = 1
};

class NullPacket : public Packet {
public:
    NullPacket() = default;
    explicit NullPacket(NullType type)
        : type_(type) {}
    ~NullPacket() override = default;

    size_t getRESP2EncodeSize() const override {
        // *-1\r\n OR $-1\r\n
        return 5;
    }

    DState encodeRESP2(Buffer *buf) override {
        if (type_ == NullType::null_bulk) {
            buf->appendInt8(BULK_STRING_PACKET_MAGIC);
        } else {
            buf->appendInt8(ARRAY_PACKET_MAGIC);
        }
        buf->append("-1", 2);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState decodeRESP2(Buffer *buf UNUSED) override {
        // not support from RESP2Codec
        return DState::ERROR;
    }

    size_t getRESP3EncodeSize() const override {
        // _\r\n
        return 3;
    }

    DState encodeRESP3(Buffer *buf) override {
        buf->appendInt8(NULL_PACKET_MAGIC);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState decodeRESP3(Buffer *buf) override {
        if (unlikely(buf->size() < 3)) {
            return DState::AGAIN;
        }
        if (unlikely(buf->peekInt8() != NULL_PACKET_MAGIC)) {
            err_ = fmt::format("Protocol error: expected '_', got '{}'", (char)buf->peekInt8());
            return DState::ERROR;
        }
        const char *start = buf->data() + 1; // skip magic
        if (unlikely(start[0] != '\r' || start[1] != '\n')) {
            err_ = fmt::format("Protocol error: expected <CR><LF>, got '{}''{}'", start[0], start[1]);
            return DState::ERROR;
        }
        buf->skip(3);
        return DState::SUCCESS;
    }

protected:
    NullType type_ = NullType::null_bulk;
};

} // namespace tair::protocol
