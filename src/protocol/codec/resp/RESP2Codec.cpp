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
#include "protocol/codec/resp/RESP2Codec.hpp"

#include "protocol/ProtocolOptions.hpp"
#include "protocol/packet/resp/ArrayPacket.hpp"
#include "protocol/packet/resp/RESPPacketFactory.hpp"

namespace tair::protocol {

using common::StringUtil;

// ---- for server ----

DState RESP2Codec::decodeRequest(Buffer *buf, PacketUniqPtr &packet) {
    err_.clear();
    if (buf->empty()) {
        return DState::AGAIN;
    }
    if (!req_packet_) {
        if (buf->peekInt8() == ARRAY_PACKET_MAGIC) {
            return processMultibulk(buf, packet);
        } else {
            return processInline(buf, packet);
        }
    }
    return processMultibulk(buf, packet);
}

DState RESP2Codec::encodeResponse(Buffer *buf, Packet *packet) {
    return packet->encodeRESP2(buf);
}

DState RESP2Codec::processInline(Buffer *buf, PacketUniqPtr &packet) {
    const char *newline = buf->findEOL();
    if (!newline) {
        if (buf->length() > ProtocolOptions::PROTO_RESP_INLINE_MAX_SIZE) {
            err_ = "Protocol error: too big inline request";
            return DState::ERROR;
        }
        return DState::AGAIN;
    }
    int linefeed_chars = 1; // '\n' length
    // Handle the \r\n case
    if (newline > buf->data() && *(newline - 1) == '\r') {
        newline--;
        linefeed_chars++;
    }
    std::vector<std::string> argv;
    if (!StringUtil::splitArgs(buf->data(), newline, argv)) {
        err_ = "Protocol error: unbalanced quotes in request";
        return DState::ERROR;
    }
    size_t querylen = newline - buf->data();
    size_t packet_len = querylen + linefeed_chars;
    buf->skip(packet_len);
    packet = std::make_unique<ArrayPacket>(std::move(argv));
    packet->setPacketSize(packet_len);

    return DState::SUCCESS;
}

DState RESP2Codec::processMultibulk(Buffer *buf, PacketUniqPtr &packet) {
    if (!req_packet_) {
        runtimeAssert(buf->peekInt8() == ARRAY_PACKET_MAGIC);
        auto array_packet = std::make_unique<ArrayPacket>();
        array_packet->setDecodeSizeLimit(ProtocolOptions::PROTO_RESP_DECODE_REQUEST_SIZE_LIMIT);
        req_packet_ = std::move(array_packet);
    }
    auto pre_size = buf->size();
    auto dstate = req_packet_->decodeRESP2(buf);
    if (dstate == DState::SUCCESS) {
        req_packet_->addPacketSize(pre_size - buf->size());
        packet = std::move(req_packet_);
    } else if (dstate == DState::AGAIN) {
        req_packet_->addPacketSize(pre_size - buf->size());
    } else if (dstate == DState::ERROR) {
        err_ = req_packet_->getDecodeErr();
    }
    return dstate;
}

// ---- for client ----

DState RESP2Codec::encodeRequest(Buffer *buf, Packet *packet) {
    return packet->encodeRESP2(buf);
}

DState RESP2Codec::decodeResponse(Buffer *buf, PacketUniqPtr &packet) {
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
    auto dstate = resp_packet_->decodeRESP2(buf);
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
