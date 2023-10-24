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

#include "protocol/packet/Packet.hpp"

namespace tair::protocol {

class BooleanPacket : public Packet {
public:
    BooleanPacket() = default;
    explicit BooleanPacket(bool val)
        : val_(val) {}
    ~BooleanPacket() override = default;

    bool getValue() const {
        return val_;
    }

    size_t getRESP2EncodeSize() const override {
        // :1\r\n or :0\r\n
        return 4;
    }

    DState encodeRESP2(Buffer *buf) override {
        // :1\r\n or :0\r\n
        buf->appendInt8(INTEGER_PACKET_MAGIC);
        buf->appendNumberToStr(val_ ? 1 : 0);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState decodeRESP2(Buffer *buf UNUSED) override {
        // not support from RESP2Codec
        return DState::ERROR;
    }

    size_t getRESP3EncodeSize() const override {
        // #t\r\n or #f\r\n
        return 4;
    }

    DState encodeRESP3(Buffer *buf) override {
        buf->appendInt8(BOOLEAN_PACKET_MAGIC);
        buf->append(val_ ? "t" : "f");
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState decodeRESP3(Buffer *buf) override {
        if (buf->size() < 4) {
            return DState::AGAIN;
        }
        if (buf->peekInt8() != BOOLEAN_PACKET_MAGIC) {
            err_ = fmt::format("Protocol error: expected '#', got '{}'", (char)buf->peekInt8());
            return DState::ERROR;
        }
        const char *start = buf->data() + 1; // skip magic
        if (start[0] != 't' && start[0] != 'f') {
            err_ = fmt::format("Protocol error: expected 't' or 'f', got '{}'", start[0]);
            return DState::ERROR;
        }
        if (start[1] != '\r' || start[2] != '\n') {
            err_ = fmt::format("Protocol error: expected <CR><LF>, got '{}''{}'", start[1], start[2]);
            return DState::ERROR;
        }
        val_ = (start[0] == 't');
        buf->skip(4);
        return DState::SUCCESS;
    }

protected:
    bool val_ = false;
};

} // namespace tair::protocol
