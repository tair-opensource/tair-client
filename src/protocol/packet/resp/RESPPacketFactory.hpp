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

#include "protocol/packet/Packet.hpp"
#include "protocol/packet/resp/BooleanPacket.hpp"
#include "protocol/packet/resp/BulkStringPacket.hpp"
#include "protocol/packet/resp/DoublePacket.hpp"
#include "protocol/packet/resp/IntegerPacket.hpp"

namespace tair::protocol {

class RESPPacketFactory {
public:
    static PacketUniqPtr createPacket(char ch);

    template <typename T>
    static Packet *createPacket(T &&t) {
        if constexpr (std::is_same_v<bool, std::remove_reference_t<T>>) {
            return new BooleanPacket(t);
        } else if constexpr (std::is_integral_v<std::remove_reference_t<T>>) {
            return new IntegerPacket(t);
        } else if constexpr (std::is_floating_point_v<std::remove_reference_t<T>>) {
            return new DoublePacket(t);
        } else {
            return new BulkStringPacket(std::forward<T>(t));
        }
    }
};

} // namespace tair::protocol
