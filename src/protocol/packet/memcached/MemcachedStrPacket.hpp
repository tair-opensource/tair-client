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

class MemcachedStrPacket : public Packet {
public:
    MemcachedStrPacket(const std::string &str, const std::any &conext)
        : str_(str), context_(conext) {}

    DState encodeMemcachedText(Buffer *buf) override {
        buf->append(str_);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState encodeMemcachedBinary(Buffer *buf) override {
        auto context = std::any_cast<MemcachedPacketContext>(context_);
        MemcachedBinRespHeader resp_str;

        resp_str.magic = MEMCACHED_BINARY_RESP_FLAG;
        resp_str.opcode = context.opcode;
        resp_str.keylen = 0;
        resp_str.extlen = 0;
        resp_str.datatype = MEMCACHED_BINARY_DATA_TYPE_RAW_BYTES;
        resp_str.status = 0;
        resp_str.bodylen = htonu32(str_.size());
        resp_str.opaque = htonu32(context.opaque);
        resp_str.cas = htonu64(context.cas);

        buf->append(&resp_str, sizeof(resp_str));
        buf->append(str_);

        return DState::SUCCESS;
    }

    size_t getRESP2EncodeSize() const override {
        // + and \r\n
        return 1 + str_.size() + 2;
    }
    DState encodeRESP2(Buffer *buf) override {
        buf->appendInt8(SIMPLE_STRING_PACKET_MAGIC);
        buf->append(str_);
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
    std::string str_;
    std::any context_;
};
} // namespace tair::protocol
