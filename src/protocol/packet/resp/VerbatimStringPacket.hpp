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

#include "protocol/packet/resp/BulkStringPacket.hpp"

namespace tair::protocol {

class VerbatimStringPacket : public BulkStringPacket {
public:
    VerbatimStringPacket(const std::string &str, const std::string &ext) {
        bulk_str_ = str;
        ext_ = ext;
    }
    VerbatimStringPacket() = default;
    ~VerbatimStringPacket() override = default;

    const std::string &getExt() const {
        return ext_;
    }

    size_t getRESP2EncodeSize() const override {
        return BulkStringPacket::getEncodeSize();
    }

    DState encodeRESP2(Buffer *buf) override {
        return BulkStringPacket::encode(buf, BULK_STRING_PACKET_MAGIC);
    }

    DState decodeRESP2(Buffer *buf) override {
        // not support from RESP2Codec
        return DState::ERROR;
    }

    size_t getRESP3EncodeSize() const override {
        // = and len and \r\n and ext:(4) and bulk len and \r\n
        return 1 + fmt::formatted_size("{}", bulk_str_.size() + 4) + 2 + bulk_str_.size() + 4 + 2;
    }

    DState encodeRESP3(Buffer *buf) override {
        std::string prefix;
        if (ext_.size() >= 3) {
            prefix = ext_.substr(0, 3) + ":";
        } else {
            prefix = "   :";
        }
        buf->appendInt8(VERBATIM_STRING_PACKET_MAGIC);
        buf->appendNumberToStr(bulk_str_.size() + 4);
        buf->appendCRLF();
        buf->append(prefix);
        buf->append(bulk_str_);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState decodeRESP3(Buffer *buf) override {
        auto state = BulkStringPacket::decode(buf, VERBATIM_STRING_PACKET_MAGIC);
        if (state == DState::SUCCESS) {
            if (bulk_str_.size() < 4 || type_ == PacketType::TYPE_NULL) {
                return DState::ERROR;
            }
            ext_ = bulk_str_.substr(0, 3);
            bulk_str_.erase(0, 4); // include :
        }
        return state;
    }

protected:
    std::string ext_;
};

} // namespace tair::protocol
