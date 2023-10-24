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
#include "protocol/codec/resp/RESP3Codec.hpp"

#include "protocol/packet/resp/RESPPacketFactory.hpp"

namespace tair::protocol {

using common::StringUtil;

#define PROTO_INLINE_MAX_SIZE (1024 * 64)
#define PROTO_MBULK_BIG_ARG   (1024 * 32)

// ---- for server ----

DState RESP3Codec::encodeResponse(Buffer *buf, Packet *packet) {
    return packet->encodeRESP3(buf);
}

DState RESP3Codec::decodeRequest(Buffer *buf, PacketUniqPtr &packet) {
    return RESP2Codec::decodeRequest(buf, packet);
}

// ---- for client ----

DState RESP3Codec::encodeRequest(Buffer *buf, Packet *packet) {
    return RESP2Codec::encodeRequest(buf, packet);
}

DState RESP3Codec::decodeResponse(Buffer *buf, PacketUniqPtr &packet) {
    err_.clear();
    if (buf->empty()) {
        return DState::AGAIN;
    }
    if (!resp_packet_) {
        char ch = buf->peekInt8();
        resp_packet_ = RESPPacketFactory::createPacket(ch);
        if (!resp_packet_) {
            auto out = StringUtil::toPrintableStr(std::string(1, ch));
            err_ = fmt::format("Protocol error: unknown type '{}'", out);
            return DState::ERROR;
        }
    }
    auto pre_size = buf->size();
    auto dstate = resp_packet_->decodeRESP3(buf);
    if (dstate == DState::SUCCESS) {
        resp_packet_->addPacketSize(pre_size - buf->size());
        packet = std::move(resp_packet_);
    } else if (dstate == DState::AGAIN) {
        resp_packet_->addPacketSize(pre_size - buf->size());
    } else if (dstate == DState::ERROR) {
        err_ = resp_packet_->getDecodeErr();
    }
    return dstate;
}

} // namespace tair::protocol
