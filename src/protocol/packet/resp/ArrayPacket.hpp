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
#include "protocol/packet/resp/ErrorPacket.hpp"
#include "protocol/packet/resp/IntegerPacket.hpp"
#include "protocol/packet/resp/NullPacket.hpp"
#include "protocol/packet/resp/RESPPacketFactory.hpp"
#include "protocol/packet/resp/SimpleStringPacket.hpp"

namespace tair::protocol {

class SetPacket;
class MapPacket;

class ArrayPacket : public Packet {
public:
    template <typename T>
    ArrayPacket(T &&t) {
        addReplyItems(std::forward<T>(t));
    }

    ArrayPacket() = default;
    ArrayPacket(PacketType type)
        : type_(type) {}

    ~ArrayPacket() override {
        for (auto packet : packet_array_) {
            delete packet;
        }
        packet_array_.clear();
    }

    PacketType getType() const {
        return type_;
    }

    const std::deque<Packet *> &getPacketArray() const {
        return packet_array_;
    }

    bool moveBulks(std::vector<std::string> &bulks) {
        bulks.reserve(packet_array_.size());
        for (auto packet : packet_array_) {
            BulkStringPacket *bulk_packet = packet->packet_cast<BulkStringPacket>();
            if (bulk_packet && bulk_packet->getType() == PacketType::TYPE_COMMON) {
                bulks.emplace_back(bulk_packet->moveBulkStr());
            } else {
                return false;
            }
        }
        return true;
    }

    bool moveIntegers(std::vector<int64_t> &integers) {
        integers.reserve(packet_array_.size());
        for (auto packet : packet_array_) {
            IntegerPacket *integer_packet = packet->packet_cast<IntegerPacket>();
            if (integer_packet) {
                integers.emplace_back(integer_packet->getValue());
            } else {
                return false;
            }
        }
        return true;
    }

    template <typename T>
    inline void addReplyError(T &&t) {
        addPacket(new ErrorPacket(std::forward<T>(t)));
    }

    template <typename T>
    inline void addReplyStatus(T &&t) {
        addPacket(new SimpleStringPacket(std::forward<T>(t)));
    }

    inline void addReplyNull() {
        addPacket(new NullPacket(NullType::null_bulk));
    }

    inline void addReplyNullArray() {
        addPacket(new NullPacket(NullType::null_multi_bulk));
    }

    template <typename T>
    inline void addReplyBulk(T &&t) {
        addPacket(new BulkStringPacket(std::forward<T>(t)));
    }

    template <typename T>
    inline void addReplyBulks(T &&ts) {
        addReplyItems(std::forward<T>(ts));
    }

    template <typename T>
    inline void addReplyIntegers(T &&ts) {
        addReplyItems(std::forward<T>(ts));
    }

    template <typename T>
    inline void addReplyItems(T &&ts) {
        for (auto &&t : ts) {
            if constexpr (std::is_lvalue_reference_v<T>) {
                addPacket(RESPPacketFactory::createPacket(t));
            } else {
                addPacket(RESPPacketFactory::createPacket(std::move(t)));
            }
        }
    }

    inline void addReplyInteger(int64_t i) {
        addPacket(new IntegerPacket(i));
    }

    template <typename T>
    inline void addReplyIntegers(T &ts) {
        for (auto &t : ts) {
            static_assert(std::is_integral_v<decltype(t)>);
            addReplyInteger((int64_t)t);
        }
    }

    inline void addReplyDouble(double d) {
        addPacket(new DoublePacket(d));
    }

    template <typename T>
    inline void addReplyArray(T &&t) {
        addPacket(new ArrayPacket(t));
    }

    ArrayPacket *startNewArray() {
        ArrayPacket *array = new ArrayPacket();
        addPacket(array);
        return array;
    }

    SetPacket *startNewSet();
    MapPacket *startNewMap();

    inline void addPackets(const std::vector<Packet *> &packets) {
        packet_array_.insert(packet_array_.end(), packets.begin(), packets.end());
    }

    inline void addPacket(Packet *packet) {
        packet_array_.emplace_back(packet);
    }

    void setDecodeSizeLimit(int64_t limit) {
        decode_size_limit_ = limit;
    }

    bool isQuiet() const {
        return quiet_;
    }
    void setQuiet(bool quiet) {
        quiet_ = quiet;
    }

    const std::any &getContext() {
        return context_;
    }

    void setContext(std::any context) {
        context_ = context;
    }

    size_t getRESP2EncodeSize() const override {
        return getEncode2Size();
    }

    DState encodeRESP2(Buffer *buf) override {
        return encode2(buf, ARRAY_PACKET_MAGIC);
    }

    DState decodeRESP2(Buffer *buf) override {
        return decode2(buf, ARRAY_PACKET_MAGIC);
    }

    size_t getRESP3EncodeSize() const override {
        return getEncode3Size();
    }

    DState encodeRESP3(Buffer *buf) override {
        return encode3(buf, ARRAY_PACKET_MAGIC);
    }

    DState decodeRESP3(Buffer *buf) override {
        return decode3(buf, ARRAY_PACKET_MAGIC);
    }

protected:
    size_t getEncode2Size() const;
    DState encode2(Buffer *buf, uint8_t packet_magic);
    DState decode2(Buffer *buf, uint8_t packet_magic);
    size_t getEncode3Size() const;
    DState encode3(Buffer *buf, uint8_t packet_magic);
    DState decode3(Buffer *buf, uint8_t packet_magic);

private:
    size_t getEncodeSize(CodecType type) const;
    DState encode(Buffer *buf, uint8_t packet_magic, CodecType type);
    DState decode(Buffer *buf, uint8_t packet_magic, CodecType type);

protected:
    PacketType type_ = PacketType::TYPE_COMMON;
    std::deque<Packet *> packet_array_;
    int64_t decode_array_size_ = NOT_SET_SIZE;
    int64_t decode_size_limit_ = 0;
    bool quiet_ = false;
    std::any context_;
    PacketUniqPtr curr_parse_packet_;
};

} // namespace tair::protocol
