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

#include <memory>

#include "common/Compiler.hpp"
#include "common/Noncopyable.hpp"
#include "protocol/codec/memcached/MemcachedCodecAble.hpp"
#include "protocol/codec/resp/RESP2CodecAble.hpp"
#include "protocol/codec/resp/RESP3CodecAble.hpp"
#include "protocol/codec/resp/RESPProtocol.hpp"

namespace tair::protocol {

using common::Noncopyable;

class Packet;
using PacketPtr = std::shared_ptr<Packet>;
using PacketUniqPtr = std::unique_ptr<Packet>;

enum class PacketType : uint8_t {
    TYPE_COMMON = 0,
    TYPE_NULL = 1,
};

class Packet : private Noncopyable, public RESP2CodecAble, public RESP3CodecAble, public MemcachedCodecAble {
public:
    Packet() = default;
    virtual ~Packet() = default;

    template <typename T>
    bool instance_of() {
        return typeid(*this) == typeid(T);
    }

    template <typename T>
    T *packet_cast(void) {
        // cast to explicit type
        return instance_of<T>() ? static_cast<T *>(this) : nullptr;
    }

    inline size_t getPacketSize() const {
        return packet_size_;
    }

    inline void setPacketSize(size_t packet_size) {
        packet_size_ = packet_size;
    }

    inline void addPacketSize(size_t packet_size) {
        packet_size_ += packet_size;
    }

private:
    size_t packet_size_ = 0;
};

} // namespace tair::protocol
