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

#include "common/StringUtil.hpp"
#include "protocol/packet/resp/BulkStringPacket.hpp"

namespace tair::protocol {

using common::StringUtil;

class BlobErrorPacket : public BulkStringPacket {
public:
    template <typename T>
    BlobErrorPacket(T &&t)
        : BulkStringPacket(std::forward<T>(t)) {}

    explicit BlobErrorPacket(const std::string *str)
        : BulkStringPacket(*str) {}
    BlobErrorPacket() = default;
    ~BlobErrorPacket() override = default;

    size_t getRESP2EncodeSize() const override {
        std::string encoded_err_str;
        StringUtil::toPrintableStr(encoded_err_str, bulk_str_);
        // - and \r\n
        return 1 + encoded_err_str.size() + 2;
    }

    DState encodeRESP2(Buffer *buf) override {
        std::string encoded_err_str;
        StringUtil::toPrintableStr(encoded_err_str, bulk_str_);
        buf->appendInt8(ERROR_PACKET_MAGIC);
        buf->append(encoded_err_str);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState decodeRESP2(Buffer *buf) override {
        // not support from RESP2Codec
        return DState::ERROR;
    }

    size_t getRESP3EncodeSize() const override {
        return BulkStringPacket::getEncodeSize();
    }

    DState encodeRESP3(Buffer *buf) override {
        return BulkStringPacket::encode(buf, BLOB_ERROR_PACKET_MAGIC);
    }

    DState decodeRESP3(Buffer *buf) override {
        auto state = BulkStringPacket::decode(buf, BLOB_ERROR_PACKET_MAGIC);
        if (state == DState::SUCCESS && type_ == PacketType::TYPE_NULL) {
            return DState::ERROR;
        }
        return state;
    }
};

} // namespace tair::protocol
