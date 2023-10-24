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
#include <memory>

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
#include "protocol/packet/resp/SetPacket.hpp"
#include "protocol/packet/resp/VerbatimStringPacket.hpp"

using tair::protocol::Packet;
using tair::protocol::IntegerPacket;
using tair::protocol::SimpleStringPacket;
using tair::protocol::ErrorPacket;
using tair::protocol::BulkStringPacket;
using tair::protocol::ArrayPacket;
using tair::protocol::NullPacket;
using tair::protocol::DoublePacket;
using tair::protocol::BooleanPacket;
using tair::protocol::BlobErrorPacket;
using tair::protocol::VerbatimStringPacket;
using tair::protocol::BigNumberPacket;
using tair::protocol::MapPacket;
using tair::protocol::SetPacket;
using tair::protocol::AttributePacket;
using tair::protocol::PushPacket;

TEST(COMMON_PACKET_TEST, ONLY_TEST) {
    auto integer = std::make_unique<IntegerPacket>(10);
    ASSERT_TRUE(integer->instance_of<IntegerPacket>());
    ASSERT_FALSE(integer->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(integer->instance_of<ErrorPacket>());
    ASSERT_FALSE(integer->instance_of<BulkStringPacket>());
    ASSERT_FALSE(integer->instance_of<ArrayPacket>());
    ASSERT_FALSE(integer->instance_of<NullPacket>());
    ASSERT_FALSE(integer->instance_of<DoublePacket>());
    ASSERT_FALSE(integer->instance_of<BooleanPacket>());
    ASSERT_FALSE(integer->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(integer->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(integer->instance_of<BigNumberPacket>());
    ASSERT_FALSE(integer->instance_of<MapPacket>());
    ASSERT_FALSE(integer->instance_of<SetPacket>());
    ASSERT_FALSE(integer->instance_of<AttributePacket>());
    ASSERT_FALSE(integer->instance_of<PushPacket>());
    ASSERT_EQ(integer.get(), integer->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<PushPacket>());

    auto simpleString = std::make_unique<SimpleStringPacket>("simple");
    ASSERT_FALSE(simpleString->instance_of<IntegerPacket>());
    ASSERT_TRUE(simpleString->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(simpleString->instance_of<ErrorPacket>());
    ASSERT_FALSE(simpleString->instance_of<BulkStringPacket>());
    ASSERT_FALSE(simpleString->instance_of<ArrayPacket>());
    ASSERT_FALSE(integer->instance_of<NullPacket>());
    ASSERT_FALSE(integer->instance_of<DoublePacket>());
    ASSERT_FALSE(integer->instance_of<BooleanPacket>());
    ASSERT_FALSE(integer->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(integer->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(integer->instance_of<BigNumberPacket>());
    ASSERT_FALSE(integer->instance_of<MapPacket>());
    ASSERT_FALSE(integer->instance_of<SetPacket>());
    ASSERT_FALSE(integer->instance_of<AttributePacket>());
    ASSERT_FALSE(integer->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, simpleString->packet_cast<IntegerPacket>());
    ASSERT_EQ(simpleString.get(), simpleString->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, simpleString->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, simpleString->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, simpleString->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<PushPacket>());

    auto error = std::make_unique<ErrorPacket>("error");
    ASSERT_FALSE(error->instance_of<IntegerPacket>());
    ASSERT_FALSE(error->instance_of<SimpleStringPacket>());
    ASSERT_TRUE(error->instance_of<ErrorPacket>());
    ASSERT_FALSE(error->instance_of<BulkStringPacket>());
    ASSERT_FALSE(error->instance_of<ArrayPacket>());
    ASSERT_FALSE(integer->instance_of<NullPacket>());
    ASSERT_FALSE(integer->instance_of<DoublePacket>());
    ASSERT_FALSE(integer->instance_of<BooleanPacket>());
    ASSERT_FALSE(integer->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(integer->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(integer->instance_of<BigNumberPacket>());
    ASSERT_FALSE(integer->instance_of<MapPacket>());
    ASSERT_FALSE(integer->instance_of<SetPacket>());
    ASSERT_FALSE(integer->instance_of<AttributePacket>());
    ASSERT_FALSE(integer->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, error->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, error->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(error.get(), error->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, error->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, error->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<PushPacket>());

    auto bulkString = std::make_unique<BulkStringPacket>("bulk");
    ASSERT_FALSE(bulkString->instance_of<IntegerPacket>());
    ASSERT_FALSE(bulkString->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(bulkString->instance_of<ErrorPacket>());
    ASSERT_TRUE(bulkString->instance_of<BulkStringPacket>());
    ASSERT_FALSE(bulkString->instance_of<ArrayPacket>());
    ASSERT_FALSE(integer->instance_of<NullPacket>());
    ASSERT_FALSE(integer->instance_of<DoublePacket>());
    ASSERT_FALSE(integer->instance_of<BooleanPacket>());
    ASSERT_FALSE(integer->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(integer->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(integer->instance_of<BigNumberPacket>());
    ASSERT_FALSE(integer->instance_of<MapPacket>());
    ASSERT_FALSE(integer->instance_of<SetPacket>());
    ASSERT_FALSE(integer->instance_of<AttributePacket>());
    ASSERT_FALSE(integer->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, bulkString->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, bulkString->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, bulkString->packet_cast<ErrorPacket>());
    ASSERT_EQ(bulkString.get(), bulkString->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, bulkString->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<PushPacket>());

    auto array = std::make_unique<ArrayPacket>();
    ASSERT_FALSE(array->instance_of<IntegerPacket>());
    ASSERT_FALSE(array->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(array->instance_of<ErrorPacket>());
    ASSERT_FALSE(array->instance_of<BulkStringPacket>());
    ASSERT_TRUE(array->instance_of<ArrayPacket>());
    ASSERT_FALSE(integer->instance_of<NullPacket>());
    ASSERT_FALSE(integer->instance_of<DoublePacket>());
    ASSERT_FALSE(integer->instance_of<BooleanPacket>());
    ASSERT_FALSE(integer->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(integer->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(integer->instance_of<BigNumberPacket>());
    ASSERT_FALSE(integer->instance_of<MapPacket>());
    ASSERT_FALSE(integer->instance_of<SetPacket>());
    ASSERT_FALSE(integer->instance_of<AttributePacket>());
    ASSERT_FALSE(integer->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, array->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, array->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, array->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, array->packet_cast<BulkStringPacket>());
    ASSERT_EQ(array.get(), array->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, integer->packet_cast<PushPacket>());

    auto null = std::make_unique<NullPacket>();
    ASSERT_FALSE(null->instance_of<IntegerPacket>());
    ASSERT_FALSE(null->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(null->instance_of<ErrorPacket>());
    ASSERT_FALSE(null->instance_of<BulkStringPacket>());
    ASSERT_FALSE(null->instance_of<ArrayPacket>());
    ASSERT_TRUE(null->instance_of<NullPacket>());
    ASSERT_FALSE(null->instance_of<DoublePacket>());
    ASSERT_FALSE(null->instance_of<BooleanPacket>());
    ASSERT_FALSE(null->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(null->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(null->instance_of<BigNumberPacket>());
    ASSERT_FALSE(null->instance_of<MapPacket>());
    ASSERT_FALSE(null->instance_of<SetPacket>());
    ASSERT_FALSE(null->instance_of<AttributePacket>());
    ASSERT_FALSE(null->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<ArrayPacket>());
    ASSERT_EQ(null.get(), null->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, null->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, null->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, null->packet_cast<PushPacket>());

    auto doublePacket = std::make_unique<DoublePacket>();
    ASSERT_FALSE(doublePacket->instance_of<IntegerPacket>());
    ASSERT_FALSE(doublePacket->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(doublePacket->instance_of<ErrorPacket>());
    ASSERT_FALSE(doublePacket->instance_of<BulkStringPacket>());
    ASSERT_FALSE(doublePacket->instance_of<ArrayPacket>());
    ASSERT_FALSE(doublePacket->instance_of<NullPacket>());
    ASSERT_TRUE(doublePacket->instance_of<DoublePacket>());
    ASSERT_FALSE(doublePacket->instance_of<BooleanPacket>());
    ASSERT_FALSE(doublePacket->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(doublePacket->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(doublePacket->instance_of<BigNumberPacket>());
    ASSERT_FALSE(doublePacket->instance_of<MapPacket>());
    ASSERT_FALSE(doublePacket->instance_of<SetPacket>());
    ASSERT_FALSE(doublePacket->instance_of<AttributePacket>());
    ASSERT_FALSE(doublePacket->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<NullPacket>());
    ASSERT_EQ(doublePacket.get(), doublePacket->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, doublePacket->packet_cast<PushPacket>());

    auto boolean = std::make_unique<BooleanPacket>();
    ASSERT_FALSE(boolean->instance_of<IntegerPacket>());
    ASSERT_FALSE(boolean->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(boolean->instance_of<ErrorPacket>());
    ASSERT_FALSE(boolean->instance_of<BulkStringPacket>());
    ASSERT_FALSE(boolean->instance_of<ArrayPacket>());
    ASSERT_FALSE(boolean->instance_of<NullPacket>());
    ASSERT_FALSE(boolean->instance_of<DoublePacket>());
    ASSERT_TRUE(boolean->instance_of<BooleanPacket>());
    ASSERT_FALSE(boolean->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(boolean->instance_of<BigNumberPacket>());
    ASSERT_FALSE(boolean->instance_of<MapPacket>());
    ASSERT_FALSE(boolean->instance_of<SetPacket>());
    ASSERT_FALSE(boolean->instance_of<AttributePacket>());
    ASSERT_FALSE(boolean->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<DoublePacket>());
    ASSERT_EQ(boolean.get(), boolean->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, boolean->packet_cast<PushPacket>());

    auto blobError = std::make_unique<BlobErrorPacket>();
    ASSERT_FALSE(blobError->instance_of<IntegerPacket>());
    ASSERT_FALSE(blobError->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(blobError->instance_of<ErrorPacket>());
    ASSERT_FALSE(blobError->instance_of<BulkStringPacket>());
    ASSERT_FALSE(blobError->instance_of<ArrayPacket>());
    ASSERT_FALSE(blobError->instance_of<NullPacket>());
    ASSERT_FALSE(blobError->instance_of<DoublePacket>());
    ASSERT_FALSE(blobError->instance_of<BooleanPacket>());
    ASSERT_TRUE(blobError->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(blobError->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(blobError->instance_of<BigNumberPacket>());
    ASSERT_FALSE(blobError->instance_of<MapPacket>());
    ASSERT_FALSE(blobError->instance_of<SetPacket>());
    ASSERT_FALSE(blobError->instance_of<AttributePacket>());
    ASSERT_FALSE(blobError->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<BooleanPacket>());
    ASSERT_EQ(blobError.get(), blobError->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, blobError->packet_cast<PushPacket>());

    auto verString = std::make_unique<VerbatimStringPacket>();
    ASSERT_FALSE(verString->instance_of<IntegerPacket>());
    ASSERT_FALSE(verString->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(verString->instance_of<ErrorPacket>());
    ASSERT_FALSE(verString->instance_of<BulkStringPacket>());
    ASSERT_FALSE(verString->instance_of<ArrayPacket>());
    ASSERT_FALSE(verString->instance_of<NullPacket>());
    ASSERT_FALSE(verString->instance_of<DoublePacket>());
    ASSERT_FALSE(verString->instance_of<BooleanPacket>());
    ASSERT_FALSE(verString->instance_of<BlobErrorPacket>());
    ASSERT_TRUE(verString->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(verString->instance_of<BigNumberPacket>());
    ASSERT_FALSE(verString->instance_of<MapPacket>());
    ASSERT_FALSE(verString->instance_of<SetPacket>());
    ASSERT_FALSE(verString->instance_of<AttributePacket>());
    ASSERT_FALSE(verString->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(verString.get(), verString->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, verString->packet_cast<PushPacket>());

    auto number = std::make_unique<BigNumberPacket>();
    ASSERT_FALSE(number->instance_of<IntegerPacket>());
    ASSERT_FALSE(number->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(number->instance_of<ErrorPacket>());
    ASSERT_FALSE(number->instance_of<BulkStringPacket>());
    ASSERT_FALSE(number->instance_of<ArrayPacket>());
    ASSERT_FALSE(number->instance_of<NullPacket>());
    ASSERT_FALSE(number->instance_of<DoublePacket>());
    ASSERT_FALSE(number->instance_of<BooleanPacket>());
    ASSERT_FALSE(number->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(number->instance_of<VerbatimStringPacket>());
    ASSERT_TRUE(number->instance_of<BigNumberPacket>());
    ASSERT_FALSE(number->instance_of<MapPacket>());
    ASSERT_FALSE(number->instance_of<SetPacket>());
    ASSERT_FALSE(number->instance_of<AttributePacket>());
    ASSERT_FALSE(number->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, number->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(number.get(), number->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, number->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, number->packet_cast<PushPacket>());

    auto map = std::make_unique<MapPacket>();
    ASSERT_FALSE(map->instance_of<IntegerPacket>());
    ASSERT_FALSE(map->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(map->instance_of<ErrorPacket>());
    ASSERT_FALSE(map->instance_of<BulkStringPacket>());
    ASSERT_FALSE(map->instance_of<ArrayPacket>());
    ASSERT_FALSE(map->instance_of<NullPacket>());
    ASSERT_FALSE(map->instance_of<DoublePacket>());
    ASSERT_FALSE(map->instance_of<BooleanPacket>());
    ASSERT_FALSE(map->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(map->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(map->instance_of<BigNumberPacket>());
    ASSERT_TRUE(map->instance_of<MapPacket>());
    ASSERT_FALSE(map->instance_of<SetPacket>());
    ASSERT_FALSE(map->instance_of<AttributePacket>());
    ASSERT_FALSE(map->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, map->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<BigNumberPacket>());
    ASSERT_EQ(map.get(), map->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, map->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, map->packet_cast<PushPacket>());

    auto set = std::make_unique<SetPacket>();
    ASSERT_FALSE(set->instance_of<IntegerPacket>());
    ASSERT_FALSE(set->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(set->instance_of<ErrorPacket>());
    ASSERT_FALSE(set->instance_of<BulkStringPacket>());
    ASSERT_FALSE(set->instance_of<ArrayPacket>());
    ASSERT_FALSE(set->instance_of<NullPacket>());
    ASSERT_FALSE(set->instance_of<DoublePacket>());
    ASSERT_FALSE(set->instance_of<BooleanPacket>());
    ASSERT_FALSE(set->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(set->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(set->instance_of<BigNumberPacket>());
    ASSERT_FALSE(set->instance_of<MapPacket>());
    ASSERT_TRUE(set->instance_of<SetPacket>());
    ASSERT_FALSE(set->instance_of<AttributePacket>());
    ASSERT_FALSE(set->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, set->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<MapPacket>());
    ASSERT_EQ(set.get(), set->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, set->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, set->packet_cast<PushPacket>());

    auto attribute = std::make_unique<AttributePacket>();
    ASSERT_FALSE(attribute->instance_of<IntegerPacket>());
    ASSERT_FALSE(attribute->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(attribute->instance_of<ErrorPacket>());
    ASSERT_FALSE(attribute->instance_of<BulkStringPacket>());
    ASSERT_FALSE(attribute->instance_of<ArrayPacket>());
    ASSERT_FALSE(attribute->instance_of<NullPacket>());
    ASSERT_FALSE(attribute->instance_of<DoublePacket>());
    ASSERT_FALSE(attribute->instance_of<BooleanPacket>());
    ASSERT_FALSE(attribute->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(attribute->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(attribute->instance_of<BigNumberPacket>());
    ASSERT_FALSE(attribute->instance_of<MapPacket>());
    ASSERT_FALSE(attribute->instance_of<SetPacket>());
    ASSERT_TRUE(attribute->instance_of<AttributePacket>());
    ASSERT_FALSE(attribute->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<SetPacket>());
    ASSERT_EQ(attribute.get(), attribute->packet_cast<AttributePacket>());
    ASSERT_EQ(nullptr, attribute->packet_cast<PushPacket>());

    auto push = std::make_unique<PushPacket>();
    ASSERT_FALSE(push->instance_of<IntegerPacket>());
    ASSERT_FALSE(push->instance_of<SimpleStringPacket>());
    ASSERT_FALSE(push->instance_of<ErrorPacket>());
    ASSERT_FALSE(push->instance_of<BulkStringPacket>());
    ASSERT_FALSE(push->instance_of<ArrayPacket>());
    ASSERT_FALSE(push->instance_of<NullPacket>());
    ASSERT_FALSE(push->instance_of<DoublePacket>());
    ASSERT_FALSE(push->instance_of<BooleanPacket>());
    ASSERT_FALSE(push->instance_of<BlobErrorPacket>());
    ASSERT_FALSE(push->instance_of<VerbatimStringPacket>());
    ASSERT_FALSE(push->instance_of<BigNumberPacket>());
    ASSERT_FALSE(push->instance_of<MapPacket>());
    ASSERT_FALSE(push->instance_of<SetPacket>());
    ASSERT_FALSE(push->instance_of<AttributePacket>());
    ASSERT_TRUE(push->instance_of<PushPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<IntegerPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<SimpleStringPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<ErrorPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<BulkStringPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<ArrayPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<NullPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<DoublePacket>());
    ASSERT_EQ(nullptr, push->packet_cast<BooleanPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<BlobErrorPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<VerbatimStringPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<BigNumberPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<MapPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<SetPacket>());
    ASSERT_EQ(nullptr, push->packet_cast<AttributePacket>());
    ASSERT_EQ(push.get(), push->packet_cast<PushPacket>());
}
