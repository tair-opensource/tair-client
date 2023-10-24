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

#include "absl/strings/numbers.h"

#include "common/Compiler.hpp"
#include "protocol/packet/Packet.hpp"

namespace tair::protocol {

using absl::SimpleAtoi;
using absl::string_view;

class IntegerPacket : public Packet {
public:
    IntegerPacket()
        : integer_(0) {}
    explicit IntegerPacket(int64_t integer)
        : integer_(integer) {}
    ~IntegerPacket() override = default;

    int64_t getValue() const {
        return integer_;
    }

    size_t getRESP2EncodeSize() const override {
        // : and \r\n
        return 1 + fmt::formatted_size("{}", integer_) + 2;
    }

    DState encodeRESP2(Buffer *buf) override {
        buf->appendInt8(INTEGER_PACKET_MAGIC);
        buf->appendNumberToStr(integer_);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState decodeRESP2(Buffer *buf) override {
        if (unlikely(buf->empty())) {
            return DState::AGAIN;
        }
        if (unlikely(buf->peekInt8() != INTEGER_PACKET_MAGIC)) {
            err_ = fmt::format("Protocol error: expected ':', got '{}'", (char)buf->peekInt8());
            return DState::ERROR;
        }
        const char *start = buf->data() + 1; // skip magic
        const char *newline = buf->findCRLF(start);
        if (!newline) {
            return DState::AGAIN;
        }
        if (unlikely(newline == start)) {
            err_ = "Protocol error: not found integer";
            return DState::ERROR;
        }
        if (unlikely(!SimpleAtoi(string_view(start, newline - start), &integer_))) {
            err_ = "Protocol error: integer format error";
            return DState::ERROR;
        }
        buf->skip(newline - buf->data() + 2); // include last \r\n
        return DState::SUCCESS;
    }

    size_t getRESP3EncodeSize() const override {
        return getRESP2EncodeSize();
    }

    DState encodeRESP3(Buffer *buf) override {
        return encodeRESP2(buf);
    }

    DState decodeRESP3(Buffer *buf) override {
        return decodeRESP2(buf);
    }

protected:
    int64_t integer_;
};

} // namespace tair::protocol
