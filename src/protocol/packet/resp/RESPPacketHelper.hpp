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

#include <string>

#include "protocol/packet/resp/ArrayPacket.hpp"

namespace tair::protocol {

class RESPPacketHelper {
public:
    template <typename PACKET, typename VALUE>
    static bool getReplyData(Packet *packet, VALUE &value) {
        auto pack = packet->packet_cast<PACKET>();
        if (!pack) {
            return false;
        }
        value = pack->getValue();
        return true;
    }

    static inline bool getReplyInteger(Packet *packet, int64_t &integer) {
        return getReplyData<IntegerPacket>(packet, integer);
    }

    static inline bool getReplyError(Packet *packet, std::string &error) {
        return getReplyData<ErrorPacket>(packet, error);
    }

    static inline bool getReplyBulkStr(Packet *packet, std::string &bulk) {
        return getReplyData<BulkStringPacket>(packet, bulk);
    }

    static inline bool getReplyStatus(Packet *packet, std::string &status) {
        return getReplyData<SimpleStringPacket>(packet, status);
    }

    static inline bool getReplyArraySize(Packet *packet, size_t &size) {
        auto array_packet = packet->packet_cast<ArrayPacket>();
        if (!array_packet) {
            return false;
        }
        size = array_packet->getPacketArray().size();
        return true;
    }

    static inline bool getReplyArrayFirstBulkStr(Packet *packet, std::string &bulk) {
        auto array_packet = packet->packet_cast<ArrayPacket>();
        if (!array_packet) {
            return false;
        }
        auto packets = array_packet->getPacketArray();
        if (packets.empty()) {
            return false;
        }
        auto bulk_packet = packets[0]->packet_cast<BulkStringPacket>();
        if (!bulk_packet) {
            return false;
        }
        bulk = bulk_packet->getValue();
        return true;
    }

    template <typename INNER_PACKET, typename VALUE>
    static bool getReplyData(Packet *packet, VALUE &value1, VALUE &value2) {
        auto array_packet = packet->packet_cast<ArrayPacket>();
        if (!array_packet) {
            return false;
        }
        auto packet_array = array_packet->getPacketArray();
        if (packet_array.size() != 2) {
            return false;
        }
        auto packet1 = packet_array[0]->packet_cast<INNER_PACKET>();
        auto packet2 = packet_array[1]->packet_cast<INNER_PACKET>();
        if (!packet1 || !packet2) {
            return false;
        }
        value1 = packet1->getValue();
        value2 = packet2->getValue();
        return true;
    }

    template <typename INNER_PACKET, typename VALUE>
    static bool getReplyData(Packet *packet, VALUE &value1, VALUE &value2, VALUE &value3) {
        auto array_packet = packet->packet_cast<ArrayPacket>();
        if (!array_packet) {
            return false;
        }
        auto packet_array = array_packet->getPacketArray();
        if (packet_array.size() != 3) {
            return false;
        }
        auto packet1 = packet_array[0]->packet_cast<INNER_PACKET>();
        auto packet2 = packet_array[1]->packet_cast<INNER_PACKET>();
        auto packet3 = packet_array[2]->packet_cast<INNER_PACKET>();
        if (!packet1 || !packet2 || !packet3) {
            return false;
        }
        value1 = packet1->getValue();
        value2 = packet2->getValue();
        value3 = packet3->getValue();
        return true;
    }

    template <typename INNER_PACKET, typename VALUE>
    static bool getReplyData(Packet *packet, VALUE &value1, VALUE &value2, VALUE &value3, VALUE &value4) {
        auto array_packet = packet->packet_cast<ArrayPacket>();
        if (!array_packet) {
            return false;
        }
        auto packet_array = array_packet->getPacketArray();
        if (packet_array.size() != 4) {
            return false;
        }
        auto packet1 = packet_array[0]->packet_cast<INNER_PACKET>();
        auto packet2 = packet_array[1]->packet_cast<INNER_PACKET>();
        auto packet3 = packet_array[2]->packet_cast<INNER_PACKET>();
        auto packet4 = packet_array[3]->packet_cast<INNER_PACKET>();
        if (!packet1 || !packet2 || !packet3 || !packet4) {
            return false;
        }
        value1 = packet1->getValue();
        value2 = packet2->getValue();
        value3 = packet3->getValue();
        value4 = packet4->getValue();
        return true;
    }

    template <typename... ARGS>
    static inline bool getReplyBulkStrs(Packet *packet, ARGS &...args) {
        static_assert(sizeof...(args) <= 4, "You can't have more than 4 arguments!");
        return getReplyData<BulkStringPacket>(packet, args...);
    }

    template <typename INNER_PACKET1, typename INNER_PACKET2, typename VALUE1, typename VALUE2>
    static bool getReplyData(Packet *packet, VALUE1 &value1, VALUE2 &value2) {
        auto array_packet = packet->packet_cast<ArrayPacket>();
        if (!array_packet) {
            return false;
        }
        auto packet_array = array_packet->getPacketArray();
        if (packet_array.size() != 2) {
            return false;
        }
        auto packet1 = packet_array[0]->packet_cast<INNER_PACKET1>();
        auto packet2 = packet_array[1]->packet_cast<INNER_PACKET2>();
        if (!packet1 || !packet2) {
            return false;
        }
        value1 = packet1->getValue();
        value2 = packet2->getValue();
        return true;
    }

    template <typename INNER_PACKET1, typename INNER_PACKET2, typename INNER_PACKET3,
              typename VALUE1, typename VALUE2, typename VALUE3>
    static bool getReplyData(Packet *packet, VALUE1 &value1, VALUE2 &value2, VALUE3 &value3) {
        auto array_packet = packet->packet_cast<ArrayPacket>();
        if (!array_packet) {
            return false;
        }
        auto packet_array = array_packet->getPacketArray();
        if (packet_array.size() != 3) {
            return false;
        }
        auto packet1 = packet_array[0]->packet_cast<INNER_PACKET1>();
        auto packet2 = packet_array[1]->packet_cast<INNER_PACKET2>();
        auto packet3 = packet_array[2]->packet_cast<INNER_PACKET3>();
        if (!packet1 || !packet2 || !packet3) {
            return false;
        }
        value1 = packet1->getValue();
        value2 = packet2->getValue();
        value3 = packet3->getValue();
        return true;
    }

    static inline bool getReplyBulkBulkInteger(Packet *packet, std::string &bulk1, std::string &bulk2, int64_t &integer) {
        return getReplyData<BulkStringPacket, BulkStringPacket, IntegerPacket>(packet, bulk1, bulk2, integer);
    }
};

} // namespace tair::protocol
