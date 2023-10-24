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

#include <map>

#include "protocol/packet/resp/ArrayPacket.hpp"
#include "protocol/packet/resp/BooleanPacket.hpp"
#include "protocol/packet/resp/DoublePacket.hpp"
#include "protocol/packet/resp/RESPPacketFactory.hpp"
#include "protocol/packet/resp/SetPacket.hpp"

namespace tair::protocol {

class MapPacket : public Packet {
public:
    MapPacket() = default;

    template <typename T>
    MapPacket(T &&t) {
        addReplyItems(std::forward<T>(t));
    }

    ~MapPacket() override {
        for (const auto &pair : packet_array_) {
            delete pair.first;
            delete pair.second;
        }
        packet_array_.clear();
    }

    const std::deque<std::pair<Packet *, Packet *>> &getPacketArray() const {
        return packet_array_;
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
        runtimeAssert(ts.size() % 2 == 0);
        Packet *curr_key = nullptr;
        for (auto &t : ts) {
            Packet *packet;
            if constexpr (std::is_lvalue_reference_v<T>) {
                packet = RESPPacketFactory::createPacket(t);
            } else {
                packet = RESPPacketFactory::createPacket(std::move(t));
            }
            if (!curr_key) {
                curr_key = packet;
            } else {
                addPacketPair(curr_key, packet);
                curr_key = nullptr;
                packet = nullptr;
            }
        }
    }

    template <typename T, typename U>
    inline void addReplyBulkBulk(T &&t, U &&u) {
        addPacketPair(new BulkStringPacket(std::forward<T>(t)), new BulkStringPacket(std::forward<U>(u)));
    }

    template <typename T, typename U>
    inline void addReplyBulkInteger(T &&t, U &&u) {
        addPacketPair(new BulkStringPacket(std::forward<T>(t)), new IntegerPacket(std::forward<U>(u)));
    }

    template <typename T>
    inline void addReplyBulkDouble(T &&t, double d) {
        addPacketPair(new BulkStringPacket(std::forward<T>(t)), new DoublePacket(d));
    }

    template <typename T, typename U>
    inline void addReplyBulkArray(T &&t, U &&u) {
        addPacketPair(new BulkStringPacket(std::forward<T>(t)), new ArrayPacket(std::forward<U>(u)));
    }

    template <typename T, typename U>
    inline void addReplyBulkSet(T &&t, U &&u) {
        addPacketPair(new BulkStringPacket(std::forward<T>(t)), new SetPacket(std::forward<U>(u)));
    }

    template <typename T>
    inline void addReplyBulkNull(T &&t) {
        addPacketPair(new BulkStringPacket(std::forward<T>(t)), new NullPacket());
    }

    inline void addReplyIntegerBool(int64_t i, bool b) {
        addPacketPair(new IntegerPacket(i), new BooleanPacket(b));
    }

    template <typename T>
    MapPacket *startNewMap(T &&t) {
        Packet *key = RESPPacketFactory::createPacket(std::forward<T>(t));
        MapPacket *map = new MapPacket();
        addPacketPair(key, map);
        return map;
    }

    template <typename T>
    ArrayPacket *startNewArray(T &&t) {
        Packet *key = RESPPacketFactory::createPacket(std::forward<T>(t));
        ArrayPacket *array = new ArrayPacket();
        addPacketPair(key, array);
        return array;
    }

    template <typename T>
    SetPacket *startNewSet(T &&t) {
        Packet *key = RESPPacketFactory::createPacket(std::forward<T>(t));
        SetPacket *set = new SetPacket();
        addPacketPair(key, set);
        return set;
    }

    inline void addPacketPair(Packet *key, Packet *value) {
        packet_array_.emplace_back(std::make_pair(key, value));
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
        return encode3(buf, MAP_PACKET_MAGIC);
    }

    DState decodeRESP3(Buffer *buf) override {
        return decode3(buf, MAP_PACKET_MAGIC);
    }

protected:
    size_t getEncode2Size() const;
    DState encode2(Buffer *buf, uint8_t packet_magic);
    DState decode2(Buffer *buf, uint8_t packet_magic);
    size_t getEncode3Size() const;
    DState encode3(Buffer *buf, uint8_t packet_magic);
    DState decode3(Buffer *buf, uint8_t packet_magic);

protected:
    std::deque<std::pair<Packet *, Packet *>> packet_array_;
    int64_t decode_map_size_ = NOT_SET_SIZE;
    int64_t decode_packet_count_ = NOT_SET_SIZE;
    PacketUniqPtr curr_pair_key_;
    PacketUniqPtr curr_parse_packet_;
};

} // namespace tair::protocol
