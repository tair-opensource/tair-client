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

#include <any>

#include "protocol/packet/Packet.hpp"

namespace tair::protocol {

class MemcachedIncDecPacket : public Packet {
public:
    MemcachedIncDecPacket(MemcachedStatus status, int64_t integer, const std::any &context)
        : status_(status), integer_(integer), context_(context) {
    }

    DState encodeMemcachedText(Buffer *buf) override {
        buf->appendNumberToStr(integer_);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState encodeMemcachedBinary(Buffer *buf) override {
        auto context = std::any_cast<MemcachedPacketContext>(context_);
        MemcachedBinRespIncDec resp_incdec;
        auto &header = resp_incdec.header;
        auto &body = resp_incdec.body;

        header.magic = MEMCACHED_BINARY_RESP_FLAG;
        header.opcode = context.opcode;
        header.keylen = 0;
        header.extlen = 0;
        header.datatype = MEMCACHED_BINARY_DATA_TYPE_RAW_BYTES;
        header.status = htonu16((uint16_t)status_);
        header.bodylen = htonu32(sizeof(uint64_t));
        header.opaque = htonu32(context.opaque);
        header.cas = htonu64(context.cas);
        body.value = htonu64(integer_);

        buf->append(&resp_incdec, sizeof(resp_incdec));
        return DState::SUCCESS;
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

    DState decodeRESP2(Buffer *buf) override { return DState::ERROR; }

    size_t getRESP3EncodeSize() const override {
        return getRESP2EncodeSize();
    }
    DState encodeRESP3(Buffer *buf) override {
        return encodeRESP2(buf);
    }

    DState decodeRESP3(Buffer *buf) override { return DState::ERROR; }

private:
    MemcachedStatus status_;
    uint64_t integer_;
    std::any context_;
};

} // namespace tair::protocol