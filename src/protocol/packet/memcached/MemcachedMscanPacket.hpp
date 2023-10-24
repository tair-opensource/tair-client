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
#include <deque>
#include <vector>

#include "protocol/packet/Packet.hpp"

namespace tair::protocol {

class MemcachedMscanPacket : public Packet {
public:
    MemcachedMscanPacket(std::string &&cursor, std::deque<std::string> &&keys)
        : cursor_(std::move(cursor)), scan_key_(std::move(keys)) {}

    DState encodeMemcachedText(Buffer *buf) override {
        buf->append("VALUE ");
        buf->append(cursor_);
        buf->appendCRLF();
        for (const auto &key : scan_key_) {
            buf->append(fmt::format("{}\r\n{}\r\n", key.size(), key));
        }
        buf->append("END");
        buf->appendCRLF();

        return DState::SUCCESS;
    }

    size_t getRESP2EncodeSize() const override {
        // *2 \r\n $cursor.size \r\n cursor \r\n
        size_t size = 1 + 1 + 2 + 1 + fmt::formatted_size("{}", cursor_.size()) + 2 + cursor_.size() + 2;
        // * scan_keys.size() \r\n
        size += (1 + fmt::formatted_size("{}", scan_key_.size()) + 2);

        for (const auto &key : scan_key_) {
            size += (1 + fmt::formatted_size("{}", key.size()) + 2 + key.size() + 2);
        }
        return size;
    }
    DState encodeRESP2(Buffer *buf) override {
        buf->appendInt8(ARRAY_PACKET_MAGIC);
        buf->appendNumberToStr(2);
        buf->appendCRLF();
        buf->appendInt8(BULK_STRING_PACKET_MAGIC);
        buf->appendNumberToStr(cursor_.size());
        buf->appendCRLF();
        buf->append(cursor_);
        buf->appendCRLF();
        buf->appendInt8(ARRAY_PACKET_MAGIC);
        buf->appendNumberToStr(scan_key_.size());
        buf->appendCRLF();
        for (const auto &key : scan_key_) {
            buf->appendInt8(BULK_STRING_PACKET_MAGIC);
            buf->appendNumberToStr(key.size());
            buf->appendCRLF();
            buf->append(key);
            buf->appendCRLF();
        }
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
    std::string cursor_;
    std::deque<std::string> scan_key_;
};

} // namespace tair::protocol