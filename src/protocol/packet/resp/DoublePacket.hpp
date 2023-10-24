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
#include "common/StringUtil.hpp"
#include "protocol/packet/Packet.hpp"

namespace tair::protocol {

using common::StringUtil;

class DoublePacket : public Packet {
public:
    DoublePacket() = default;
    explicit DoublePacket(double val)
        : val_(val) {}
    ~DoublePacket() override = default;

    long double getValue() const {
        return val_;
    }

    size_t getRESP2EncodeSize() const override {
        size_t val_size;
        if (std::isinf(val_)) {
            val_size = val_ > 0 ? 3 : 4;
        } else {
            val_size = fmt::formatted_size("{:.17g}", val_);
        }
        // $ and len and \r\n and data and \r\n
        return 1 + fmt::formatted_size("{}", val_size) + 2 + val_size + 2;
    }

    DState encodeRESP2(Buffer *buf) override {
        std::string str;
        if (std::isinf(val_)) {
            str = val_ > 0 ? "inf" : "-inf";
        } else {
            str = fmt::format("{:.17g}", val_);
        }
        buf->appendInt8(BULK_STRING_PACKET_MAGIC);
        buf->appendNumberToStr(str.size());
        buf->appendCRLF();
        buf->append(str);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState decodeRESP2(Buffer *buf UNUSED) override {
        // not support from RESP2Codec
        return DState::ERROR;
    }

    size_t getRESP3EncodeSize() const override {
        size_t val_size;
        if (std::isinf(val_)) {
            val_size = val_ > 0 ? 3 : 4;
        } else {
            val_size = fmt::formatted_size("{:.17g}", val_);
        }
        // , and data and \r\n
        return 1 + val_size + 2;
    }

    DState encodeRESP3(Buffer *buf) override {
        std::string str;
        if (std::isinf(val_)) {
            str = val_ > 0 ? "inf" : "-inf";
        } else {
            str = fmt::format("{:.17g}", val_);
        }
        buf->appendInt8(DOUBLE_PACKET_MAGIC);
        buf->append(str);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState decodeRESP3(Buffer *buf) override {
        if (unlikely(buf->empty())) {
            return DState::AGAIN;
        }
        if (unlikely(buf->peekInt8() != DOUBLE_PACKET_MAGIC)) {
            err_ = fmt::format("Protocol error: expected ',', got '{}'", (char)buf->peekInt8());
            return DState::ERROR;
        }
        const char *start = buf->data() + 1; // skip magic
        const char *newline = buf->findCRLF(start);
        if (!newline) {
            return DState::AGAIN;
        }
        if (unlikely(newline == start)) {
            err_ = "Protocol error: not found double";
            return DState::ERROR;
        }
        size_t len = (size_t)(newline - start);
        if (unlikely(!StringUtil::string2ld(start, len, &val_))) {
            err_ = "Protocol error: double format error";
            return DState::ERROR;
        }
        buf->skip(newline - buf->data() + 2); // include last \r\n
        return DState::SUCCESS;
    }

protected:
    long double val_ = 0;
};

} // namespace tair::protocol
