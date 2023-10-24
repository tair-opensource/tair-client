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

class BulkStringPacket : public Packet {
public:
    template <typename T>
    BulkStringPacket(T &&t)
        : bulk_str_(std::forward<T>(t)) {}

    explicit BulkStringPacket(const std::string *str)
        : bulk_str_(*str) {}
    explicit BulkStringPacket(PacketType type)
        : type_(type) {}

    BulkStringPacket() = default;
    ~BulkStringPacket() override = default;

    PacketType getType() const {
        return type_;
    }

    const std::string &getValue() const {
        return bulk_str_;
    }

    std::string moveBulkStr() {
        return std::move(bulk_str_);
    }

    size_t getRESP2EncodeSize() const override {
        return getEncodeSize();
    }

    DState encodeRESP2(Buffer *buf) override {
        return encode(buf, BULK_STRING_PACKET_MAGIC);
    }

    DState decodeRESP2(Buffer *buf) override {
        return decode(buf, BULK_STRING_PACKET_MAGIC);
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
    size_t getEncodeSize() const;
    DState encode(Buffer *buf, uint8_t packet_magic);
    DState decode(Buffer *buf, uint8_t packet_magic);

protected:
    PacketType type_ = PacketType::TYPE_COMMON;
    int64_t decode_bulk_len_ = NOT_SET_SIZE;
    std::string bulk_str_;
};

} // namespace tair::protocol
