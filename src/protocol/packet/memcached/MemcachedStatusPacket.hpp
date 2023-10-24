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

class MemcachedStatusPacket : public Packet {
public:
    MemcachedStatusPacket(MemcachedStatus status, const std::string &info, int64_t version, const std::any &conext)
        : status_(status), info_(info), version_(version), context_(conext) {}

    MemcachedStatusPacket(std::string info, std::any conext)
        : info_(std::move(info)), version_(0), context_(std::move(conext)) {
        if (info_.starts_with("CLIENT_ERROR")) {
            status_ = MemcachedStatus::RESP_EINVAL;
        } else if (info_.starts_with("SERVER_ERROR")) {
            status_ = MemcachedStatus::RESP_E2BIG;
        } else if (info_.starts_with("NOPERM") || info_.starts_with("NOAUTH")) {
            status_ = MemcachedStatus::RESP_AUTH_ERROR;
        } else if (info_.starts_with("Unknown")) {
            status_ = MemcachedStatus::RESP_UNKNOWN_COMMAND;
        } else {
            status_ = MemcachedStatus::RESP_UNKNOWN_ERROR;
        }
    }

    const std::string getValue() const {
        return statusString(status_);
    }

    DState encodeMemcachedText(Buffer *buf) override {
        buf->append(info_);
        buf->appendCRLF();
        return DState::SUCCESS;
    }

    DState encodeMemcachedBinary(Buffer *buf) override {
        std::ignore = status_;
        auto context = std::any_cast<MemcachedPacketContext>(context_);
        MemcachedBinRespHeader resp_status;

        resp_status.magic = MEMCACHED_BINARY_RESP_FLAG;
        resp_status.opcode = context.opcode;
        resp_status.keylen = 0;
        resp_status.extlen = 0;
        resp_status.datatype = MEMCACHED_BINARY_DATA_TYPE_RAW_BYTES;
        resp_status.status = htonu16((uint16_t)status_);
        resp_status.bodylen = 0;
        resp_status.opaque = htonu32(context.opaque);
        if (version_ == 0) {
            resp_status.cas = htonu64(context.cas);
        } else {
            resp_status.cas = htonu64(version_);
        }

        if (status_ != MemcachedStatus::RESP_SUCCESS) {
            resp_status.bodylen = htonu32(statusString(status_).size());
        }
        // only header
        buf->append(&resp_status, sizeof(resp_status));

        if (status_ != MemcachedStatus::RESP_SUCCESS) {
            buf->append(statusString(status_));
        }

        return DState::SUCCESS;
    }

    size_t getRESP2EncodeSize() const override {
        // +/- and \r\n
        return 1 + info_.size() + 2;
    }
    DState encodeRESP2(Buffer *buf) override {
        if (status_ == MemcachedStatus::RESP_SUCCESS) {
            buf->appendInt8(SIMPLE_STRING_PACKET_MAGIC);
        } else {
            buf->appendInt8(ERROR_PACKET_MAGIC);
        }
        buf->append(info_);
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
    static std::string statusString(MemcachedStatus status) {
        std::string errstr;
        switch (status) {
            case MemcachedStatus::RESP_ENOMEM:
                errstr = "Out of memory";
                break;
            case MemcachedStatus::RESP_UNKNOWN_COMMAND:
                errstr = "Unknown command";
                break;
            case MemcachedStatus::RESP_KEY_ENOENT:
                errstr = "Not found";
                break;
            case MemcachedStatus::RESP_EINVAL:
                errstr = "Invalid arguments";
                break;
            case MemcachedStatus::RESP_KEY_EEXISTS:
                errstr = "Data exists for key.";
                break;
            case MemcachedStatus::RESP_E2BIG:
                errstr = "Too large.";
                break;
            case MemcachedStatus::RESP_DELTA_BADVAL:
                errstr = "Non-numeric server-side value for incr or decr";
                break;
            case MemcachedStatus::RESP_NOT_STORED:
                errstr = "Not stored.";
                break;
            case MemcachedStatus::RESP_AUTH_ERROR:
                errstr = "Auth failure.";
                break;
            default:
                errstr = "UNHANDLED ERROR";
                break;
        }
        return errstr;
    }

private:
    MemcachedStatus status_;
    std::string info_;
    int64_t version_;
    std::any context_;
};

} // namespace tair::protocol