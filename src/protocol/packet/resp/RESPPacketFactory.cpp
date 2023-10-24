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
#include "protocol/packet/resp/RESPPacketFactory.hpp"

#include "protocol/packet/resp/ArrayPacket.hpp"
#include "protocol/packet/resp/AttributePacket.hpp"
#include "protocol/packet/resp/BigNumberPacket.hpp"
#include "protocol/packet/resp/BlobErrorPacket.hpp"
#include "protocol/packet/resp/MapPacket.hpp"
#include "protocol/packet/resp/NullPacket.hpp"
#include "protocol/packet/resp/PushPacket.hpp"
#include "protocol/packet/resp/SetPacket.hpp"
#include "protocol/packet/resp/VerbatimStringPacket.hpp"

namespace tair::protocol {

PacketUniqPtr RESPPacketFactory::createPacket(char ch) {
    switch (ch) {
        case ARRAY_PACKET_MAGIC: // '*'
            return std::make_unique<ArrayPacket>();
        case BULK_STRING_PACKET_MAGIC: // '$'
            return std::make_unique<BulkStringPacket>();
        case SIMPLE_STRING_PACKET_MAGIC: // '+'
            return std::make_unique<SimpleStringPacket>();
        case ERROR_PACKET_MAGIC: // '-'
            return std::make_unique<ErrorPacket>();
        case INTEGER_PACKET_MAGIC: // ':'
            return std::make_unique<IntegerPacket>();
        case NULL_PACKET_MAGIC: // '_'
            return std::make_unique<NullPacket>();
        case DOUBLE_PACKET_MAGIC: // ','
            return std::make_unique<DoublePacket>();
        case BOOLEAN_PACKET_MAGIC: // '#'
            return std::make_unique<BooleanPacket>();
        case BLOB_ERROR_PACKET_MAGIC: // '!'
            return std::make_unique<BlobErrorPacket>();
        case VERBATIM_STRING_PACKET_MAGIC: // '='
            return std::make_unique<VerbatimStringPacket>();
        case BIG_NUMBER_PACKET_MAGIC: // '('
            return std::make_unique<BigNumberPacket>();
        case MAP_PACKET_MAGIC: // '%'
            return std::make_unique<MapPacket>();
        case SET_PACKET_MAGIC: // '~'
            return std::make_unique<SetPacket>();
        case ATTRIBUTE_PACKET_MAGIC: // '|'
            return std::make_unique<AttributePacket>();
        case PUSH_PACKET_MAGIC: // '>'
            return std::make_unique<PushPacket>();
        default:
            return nullptr;
    }
}

} // namespace tair::protocol
