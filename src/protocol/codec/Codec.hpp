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
#include <memory>

#include "common/Noncopyable.hpp"
#include "network/Buffer.hpp"
#include "protocol/codec/CodecType.hpp"
#include "protocol/packet/Packet.hpp"

namespace tair::protocol {

using common::Noncopyable;
using network::Buffer;

class Codec;
using CodecPtr = std::shared_ptr<Codec>;

class Codec : private Noncopyable {
public:
    Codec() = default;
    virtual ~Codec() = default;

    // for server
    virtual DState decodeRequest(Buffer *buf, PacketUniqPtr &packet) = 0;
    virtual DState encodeResponse(Buffer *buf, Packet *packet) = 0;

    // for client
    virtual DState encodeRequest(Buffer *buf, Packet *packet) = 0;
    virtual DState decodeResponse(Buffer *buf, PacketUniqPtr &packet) = 0;

    CodecType getCodecType() const {
        return codec_ver_;
    }

    const std::string &getErr() const {
        return err_;
    }

    const std::any &getContext() { return context_; };

protected:
    CodecType codec_ver_ = CodecType::NONE;
    std::string err_;
    std::any context_;
};

} // namespace tair::protocol
