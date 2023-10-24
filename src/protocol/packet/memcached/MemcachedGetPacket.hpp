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
#include <vector>

#include "protocol/packet/Packet.hpp"

namespace tair::protocol {

struct MemcachedKeyValuePack {
    explicit MemcachedKeyValuePack(const std::string &key, const std::string &value, uint64_t version, uint32_t flags, bool value_is_null)
        : key(key), value(value), version(version), flags(flags), value_is_null(value_is_null) {}
    std::string key;
    std::string value;
    uint64_t version;
    uint32_t flags;
    bool value_is_null;
};

class MemcachedGetPacket : public Packet {
public:
    MemcachedGetPacket(std::vector<MemcachedKeyValuePack> &&packs, bool with_version, const std::any &conext)
        : key_value_packs_(std::move(packs)), with_version_(with_version), context_(conext) {}

    DState encodeMemcachedText(Buffer *buf) override {
        for (const auto &pack : key_value_packs_) {
            if (!with_version_ && !pack.value_is_null) {
                buf->append(fmt::format("VALUE {} {} {}\r\n{}\r\n", pack.key, pack.flags, pack.value.size(), pack.value));
            } else if (with_version_ && !pack.value_is_null) {
                buf->append(fmt::format("VALUE {} {} {} {}\r\n{}\r\n", pack.key, pack.flags, pack.value.size(), pack.version, pack.value));
            }
        }
        buf->append("END");
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState encodeMemcachedBinary(Buffer *buf) override {
        auto context = std::any_cast<MemcachedPacketContext>(context_);
        runtimeAssert(key_value_packs_.size() == 1);
        auto &pack = key_value_packs_[0];
        if (pack.value_is_null) {
            MemcachedBinRespHeader resp_get;

            resp_get.magic = MEMCACHED_BINARY_RESP_FLAG;
            resp_get.opcode = context.opcode;
            resp_get.keylen = 0;
            resp_get.extlen = 0;
            resp_get.datatype = MEMCACHED_BINARY_DATA_TYPE_RAW_BYTES;
            resp_get.status = htonu16((uint16_t)MemcachedStatus::RESP_KEY_ENOENT);
            resp_get.bodylen = 0;
            resp_get.opaque = htonu32(context.opaque);
            resp_get.cas = htonu64(context.cas);

            // only header
            buf->append(&resp_get, sizeof(resp_get));
        } else {
            MemcachedBinRespGet resp_get;
            auto &header = resp_get.header;
            auto &body = resp_get.body;

            header.magic = MEMCACHED_BINARY_RESP_FLAG;
            header.opcode = context.opcode;
            if (header.opcode == MEMCACHED_BINARY_CMD_GETK) {
                header.keylen = htonu16(pack.key.size());
                header.bodylen = htonu32(sizeof(resp_get.body.flags) + pack.key.size() + pack.value.size());
            } else {
                header.keylen = 0;
                header.bodylen = htonu32(sizeof(resp_get.body.flags) + pack.value.size());
            }
            header.extlen = sizeof(resp_get.body.flags);
            header.datatype = MEMCACHED_BINARY_DATA_TYPE_RAW_BYTES;
            header.status = htonu16((uint16_t)MemcachedStatus::RESP_SUCCESS);
            header.opaque = htonu32(context.opaque);
            header.cas = htonu64(pack.version);
            body.flags = htonu32(pack.flags);

            buf->append(&resp_get, sizeof(resp_get));
            if (header.keylen != 0) {
                buf->append(pack.key);
            }
            buf->append(pack.value);
        }

        return DState::SUCCESS;
    }

    size_t getRESP2EncodeSize() const override {
        // * and number + \r\n
        size_t size = 1 + fmt::formatted_size("{}", key_value_packs_.size()) + 2;
        for (const auto &pack : key_value_packs_) {
            if (!pack.value_is_null) {
                size += (1 + fmt::formatted_size("{}", pack.value.size()) + 2 + pack.value.size() + 2);
            } else {
                // * -1 \r\n
                size += (1 + 2 + 2);
            }
        }
        return size;
    }
    DState encodeRESP2(Buffer *buf) override {
        buf->appendInt8(ARRAY_PACKET_MAGIC);
        buf->appendNumberToStr(key_value_packs_.size());
        buf->appendCRLF();
        for (const auto &pack : key_value_packs_) {
            buf->appendInt8(BULK_STRING_PACKET_MAGIC);
            if (!pack.value_is_null) {
                buf->appendNumberToStr(pack.value.size());
                buf->appendCRLF();
                buf->append(pack.value);
            } else {
                buf->append("-1", 2);
            }
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
    std::vector<MemcachedKeyValuePack> key_value_packs_;
    bool with_version_ = false;
    std::any context_;
};

} // namespace tair::protocol