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
#include "gtest/gtest.h"

#include "protocol/packet/resp/ArrayPacket.hpp"
#include "protocol/packet/resp/AttributePacket.hpp"
#include "protocol/packet/resp/BigNumberPacket.hpp"
#include "protocol/packet/resp/BlobErrorPacket.hpp"
#include "protocol/packet/resp/BooleanPacket.hpp"
#include "protocol/packet/resp/DoublePacket.hpp"
#include "protocol/packet/resp/MapPacket.hpp"
#include "protocol/packet/resp/NullPacket.hpp"
#include "protocol/packet/resp/PushPacket.hpp"
#include "protocol/packet/resp/RESPPacketFactory.hpp"
#include "protocol/packet/resp/SetPacket.hpp"
#include "protocol/packet/resp/VerbatimStringPacket.hpp"

using tair::protocol::RESPPacketFactory;
using tair::protocol::SimpleStringPacket;
using tair::protocol::ErrorPacket;
using tair::protocol::IntegerPacket;
using tair::protocol::BulkStringPacket;
using tair::protocol::ArrayPacket;
using tair::protocol::NullPacket;
using tair::protocol::DoublePacket;
using tair::protocol::BooleanPacket;
using tair::protocol::BlobErrorPacket;
using tair::protocol::VerbatimStringPacket;
using tair::protocol::BigNumberPacket;
using tair::protocol::MapPacket;
using tair::protocol::MapPacket;
using tair::protocol::SetPacket;
using tair::protocol::AttributePacket;
using tair::protocol::PushPacket;

TEST(PACKET_FACTORY_TEST, ENCODE_TEST) {
    ASSERT_TRUE(RESPPacketFactory::createPacket(SIMPLE_STRING_PACKET_MAGIC)->instance_of<SimpleStringPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(ERROR_PACKET_MAGIC)->instance_of<ErrorPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(INTEGER_PACKET_MAGIC)->instance_of<IntegerPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(BULK_STRING_PACKET_MAGIC)->instance_of<BulkStringPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(ARRAY_PACKET_MAGIC)->instance_of<ArrayPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(NULL_PACKET_MAGIC)->instance_of<NullPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(DOUBLE_PACKET_MAGIC)->instance_of<DoublePacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(BOOLEAN_PACKET_MAGIC)->instance_of<BooleanPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(BLOB_ERROR_PACKET_MAGIC)->instance_of<BlobErrorPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(VERBATIM_STRING_PACKET_MAGIC)->instance_of<VerbatimStringPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(BIG_NUMBER_PACKET_MAGIC)->instance_of<BigNumberPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(MAP_PACKET_MAGIC)->instance_of<MapPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(SET_PACKET_MAGIC)->instance_of<SetPacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(ATTRIBUTE_PACKET_MAGIC)->instance_of<AttributePacket>());
    ASSERT_TRUE(RESPPacketFactory::createPacket(PUSH_PACKET_MAGIC)->instance_of<PushPacket>());
}
