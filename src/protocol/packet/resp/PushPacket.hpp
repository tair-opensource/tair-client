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

#include "protocol/packet/resp/ArrayPacket.hpp"

namespace tair::protocol {

class PushPacket : public ArrayPacket {
public:
    template <typename T>
    PushPacket(T &&t)
        : ArrayPacket(std::forward<T>(t)) {}

    PushPacket() = default;
    ~PushPacket() override = default;

    size_t getRESP2EncodeSize() const override {
        return ArrayPacket::getEncode2Size();
    }

    DState encodeRESP2(Buffer *buf) override {
        return ArrayPacket::encode2(buf, ARRAY_PACKET_MAGIC);
    }

    DState decodeRESP2(Buffer *buf) override {
        // not support from RESP2Codec
        return DState::ERROR;
    }

    size_t getRESP3EncodeSize() const override {
        return ArrayPacket::getEncode3Size();
    }

    DState encodeRESP3(Buffer *buf) override {
        return ArrayPacket::encode3(buf, PUSH_PACKET_MAGIC);
    }

    DState decodeRESP3(Buffer *buf) override {
        auto state = ArrayPacket::decode3(buf, PUSH_PACKET_MAGIC);
        if (state == DState::SUCCESS && type_ == PacketType::TYPE_NULL) {
            return DState::ERROR;
        }
        return state;
    }
};

} // namespace tair::protocol
