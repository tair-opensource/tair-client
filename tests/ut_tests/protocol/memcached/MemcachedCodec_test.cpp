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

#include "network/Buffer.hpp"
#include "protocol/codec/CodecFactory.hpp"
#include "protocol/codec/memcached/MemcachedCodec.hpp"
#include "protocol/codec/memcached/MemcachedProtocol.hpp"
#include "protocol/packet/resp/ArrayPacket.hpp"

using tair::network::Buffer;
using tair::protocol::DState;
using tair::protocol::Codec;
using tair::protocol::CodecType;
using tair::protocol::CodecFactory;
using tair::protocol::MemcachedCodec;
using tair::protocol::PacketUniqPtr;
using tair::protocol::ArrayPacket;
using tair::protocol::MemcachedBinReqHeader;
using tair::protocol::MEMCACHED_BINARY_REQ_FLAG;
using tair::common::Endianconv;

TEST(MEMCACHED_CODEC_TEST, CODEC_VERSION_TEST) {
    auto codec = CodecFactory::getCodec(CodecType::MEMCACHED);
    ASSERT_EQ(CodecType::MEMCACHED, codec->getCodecType());
}

TEST(MEMCACHED_CODEC_TEST, TEXT_DECODE_TEST) {
    PacketUniqPtr packet;

    MemcachedCodec codec;
    Buffer buf;
    ASSERT_EQ(DState::AGAIN, codec.decodeRequest(&buf, packet));

    buf.append("k1");
    ASSERT_EQ(DState::AGAIN, codec.decodeRequest(&buf, packet));

    buf.clear();
    buf.append("\r\n");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("ERROR", codec.getErr());

    buf.clear();
    buf.append("set ");
    buf.append(std::string(300, 'a'));
    buf.append(" 0 0 1\r\n");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());

    buf.clear();
    buf.append("set k1 0 0\r\nval\r\n");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());

    buf.clear();
    buf.append("set k1 0 0 a\r\nvala\r\n");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());

    buf.clear();
    buf.append("set k1 0 0 -1\r\nvala\r\n");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());

    buf.clear();
    buf.append("set k1 0 0 1\r\n");
    ASSERT_EQ(DState::AGAIN, codec.decodeRequest(&buf, packet));

    buf.clear();
    buf.append("set k1 0 0 4\r\nval\r\n");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad data chunk", codec.getErr());

    buf.clear();
    buf.append("set k1 0 0 1048577\r\n");
    buf.append(std::string(1048577, 'a'));
    buf.append("\r\n");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("SERVER_ERROR object too large for cache", codec.getErr());

    buf.clear();
    buf.append("set k1 0 0 5\r\nvalue\r\n");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(7U, argv.size());
    ASSERT_EQ("memcache_set", argv[0]);
    ASSERT_EQ("value", argv[5]);
    ASSERT_EQ("-1", argv[6]);

    buf.clear();
    buf.append("incr k1 1\r\n");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(6U, argv.size());
    ASSERT_EQ("memcache_incr", argv[0]);
    ASSERT_EQ("0", argv[4]);
    ASSERT_EQ("-1", argv[5]);

    buf.clear();
    buf.append("append k1 0 0 4\r\nvala\r\n");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(7U, argv.size());
    ASSERT_EQ("memcache_append", argv[0]);
    ASSERT_EQ("vala", argv[5]);
    ASSERT_EQ("-1", argv[6]);

    buf.clear();
    buf.append("delete k1\r\n");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());
    ASSERT_EQ("memcache_delete", argv[0]);
    ASSERT_EQ("-1", argv[2]);

    buf.clear();
    buf.append("get k1 k2 k3\r\n");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(4U, argv.size());
    ASSERT_EQ("memcache_get", argv[0]);

    buf.clear();
    buf.append("cas k1 0 0 4 1\r\nvala\r\n");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(7U, argv.size());
    ASSERT_EQ("memcache_cas", argv[0]);
    ASSERT_EQ("vala", argv[6]);

    buf.clear();
    buf.append("mscan 0 count 1 match 1\r\n*\r\n");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(7U, argv.size());
    ASSERT_EQ("memcache_mscan", argv[0]);
    ASSERT_EQ("*", argv[6]);
}

TEST(MEMCACHED_CODEC_TEST, ERROR_DECODE_TEST) {
    PacketUniqPtr packet;
    MemcachedCodec codec;
    Buffer buf;

    MemcachedBinReqHeader header;
    header.magic = MEMCACHED_BINARY_REQ_FLAG;

    buf.append(&header, sizeof(header.magic));
    ASSERT_EQ(DState::AGAIN, codec.decodeRequest(&buf, packet));

    buf.clear();
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_SET;
    header.keylen = Endianconv::intrev16(300);
    header.extlen = 0;
    header.datatype = 0;
    header.reserved = 0;
    header.bodylen = 0;
    header.opaque = 0;
    header.cas = 0;
    buf.append(&header, sizeof(header));
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());

    buf.clear();
    header.keylen = Endianconv::intrev16(10);
    header.bodylen = Endianconv::intrev32(50);
    buf.append(&header, sizeof(header));
    ASSERT_EQ(DState::AGAIN, codec.decodeRequest(&buf, packet));

    buf.clear();
    header.opcode = 0x55;
    header.keylen = 0;
    header.bodylen = 0;
    buf.append(&header, sizeof(header));
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("Unknown Command", codec.getErr());
}

TEST(MEMCACHED_CODEC_TEST, VERSION_DECODE_TEST) {
    PacketUniqPtr packet;
    MemcachedCodec codec;
    Buffer buf;

    MemcachedBinReqHeader header;
    header.magic = MEMCACHED_BINARY_REQ_FLAG;
    header.extlen = 0;
    header.datatype = 0;
    header.reserved = 0;
    header.opaque = 0;
    header.cas = 0;
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_VERSION;
    header.keylen = Endianconv::intrev16(0);
    header.bodylen = Endianconv::intrev16(0);
    buf.append(&header, sizeof(header));
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(1U, argv.size());

    buf.clear();
    header.keylen = Endianconv::intrev16(10);
    buf.append(&header, sizeof(header));
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());
}

TEST(MEMCACHED_CODEC_TEST, FLUSHALL_DECODE_TEST) {
    PacketUniqPtr packet;
    MemcachedCodec codec;
    Buffer buf;

    tair::protocol::MemcachedBinReqFlush req;
    auto &header = req.header;
    header.magic = MEMCACHED_BINARY_REQ_FLAG;
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_FLUSH;
    header.keylen = 0;
    header.extlen = (uint8_t)4;
    header.datatype = 0;
    header.reserved = 0;
    header.bodylen = Endianconv::intrev32(4);
    header.opaque = 0;
    header.cas = 0;
    req.body.expiration = 0;

    buf.append(&req, sizeof(req));
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(1U, argv.size());

    buf.clear();
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_FLUSHQ;
    buf.append(&req, sizeof(req));
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(1U, argv.size());

    buf.clear();
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_FLUSH;
    req.body.expiration = 10;
    buf.append(&req, sizeof(req));
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(2U, argv.size());

    buf.clear();
    header.extlen = 0;
    header.bodylen = 0;
    buf.append(&req, sizeof(req.header));
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());

    buf.clear();
    header.bodylen = Endianconv::intrev32(4);
    buf.append(&req, sizeof(req));
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());
}

TEST(MEMCACHED_CODEC_TEST, SET_DECODE_TEST) {
    PacketUniqPtr packet;
    MemcachedCodec codec;
    Buffer buf;

    tair::protocol::MemcachedBinReqSet req;
    auto &header = req.header;
    auto &body = req.body;

    header.magic = MEMCACHED_BINARY_REQ_FLAG;
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_SET;
    header.keylen = Endianconv::intrev16(2);
    header.extlen = (uint8_t)8;
    header.datatype = 0;
    header.reserved = 0;
    header.bodylen = Endianconv::intrev32(12);
    header.opaque = 0;
    header.cas = 0;
    body.expiration = 0;
    body.flags = 0;

    buf.append(&req, sizeof(req));
    buf.append("k1v1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(7U, argv.size());

    buf.clear();
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_SETQ;
    buf.append(&req, sizeof(req));
    buf.append("k1v1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(7U, argv.size());

    buf.clear();
    header.keylen = 0;
    buf.append(&req, sizeof(req));
    buf.append("k1v1");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());

    buf.clear();
    header.bodylen = Endianconv::intrev32(1048588);
    header.keylen = Endianconv::intrev16(2);
    buf.append(&req, sizeof(req));
    buf.append("k1");
    buf.append(std::string(1048578, 'a'));
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("SERVER_ERROR object too large for cache", codec.getErr());
}

TEST(MEMCACHED_CODEC_TEST, APPEND_DECODE_TEST) {
    PacketUniqPtr packet;
    MemcachedCodec codec;
    Buffer buf;

    MemcachedBinReqHeader header;
    header.magic = MEMCACHED_BINARY_REQ_FLAG;
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_APPEND;
    header.keylen = Endianconv::intrev16(2);
    header.extlen = 0;
    header.datatype = 0;
    header.reserved = 0;
    header.bodylen = Endianconv::intrev32(4);
    header.opaque = 0;
    header.cas = 0;

    buf.append(&header, sizeof(header));
    buf.append("k1v1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(7U, argv.size());

    buf.clear();
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_APPENDQ;
    buf.append(&header, sizeof(header));
    buf.append("k1v1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(7U, argv.size());

    buf.clear();
    header.keylen = 0;
    buf.append(&header, sizeof(header));
    buf.append("k1v1");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
}

TEST(MEMCACHED_CODEC_TEST, DELETE_DECODE_TEST) {
    PacketUniqPtr packet;
    MemcachedCodec codec;
    Buffer buf;

    MemcachedBinReqHeader header;
    header.magic = MEMCACHED_BINARY_REQ_FLAG;
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_DELETE;
    header.keylen = Endianconv::intrev16(2);
    header.extlen = 0;
    header.datatype = 0;
    header.reserved = 0;
    header.bodylen = Endianconv::intrev32(2);
    header.opaque = 0;
    header.cas = 0;

    buf.append(&header, sizeof(header));
    buf.append("k1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());

    buf.clear();
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_DELETEQ;
    buf.append(&header, sizeof(header));
    buf.append("k1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());

    buf.clear();
    header.bodylen = Endianconv::intrev32(4);
    buf.append(&header, sizeof(header));
    buf.append("k1v1");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());
}

TEST(MEMCACHED_CODEC_TEST, INCRDECR_DECODE_TEST) {
    PacketUniqPtr packet;
    MemcachedCodec codec;
    Buffer buf;

    tair::protocol::MemcachedBinReqIncr req;
    auto &header = req.header;
    auto &body = req.body;

    header.magic = MEMCACHED_BINARY_REQ_FLAG;
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_INCREMENT;
    header.keylen = Endianconv::intrev16(2);
    header.extlen = (int8_t)20;
    header.datatype = 0;
    header.reserved = 0;
    header.bodylen = Endianconv::intrev32(22);
    header.opaque = 0;
    header.cas = 0;
    body.delta = 2;
    body.initial = 1;
    body.expiration = 0;

    buf.append(&req, sizeof(req));
    buf.append("k1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(6U, argv.size());

    buf.clear();
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_INCREMENTQ;
    buf.append(&req, sizeof(req));
    buf.append("k1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(6U, argv.size());

    buf.clear();
    header.keylen = 0;
    buf.append(&req, sizeof(req));
    buf.append("k1");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());

    buf.clear();
    header.keylen = Endianconv::intrev16(2);
    header.extlen = (uint8_t)16;
    header.bodylen = Endianconv::intrev32(18);
    buf.append(&req, sizeof(req) - 4);
    buf.append("k1");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());
}

TEST(MEMCACHED_CODEC_TEST, TOUCH_DECODE_TEST) {
    PacketUniqPtr packet;
    MemcachedCodec codec;
    Buffer buf;

    tair::protocol::MemcachedBinReqTouch req;
    auto &header = req.header;
    auto &body = req.body;
    header.magic = MEMCACHED_BINARY_REQ_FLAG;
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_TOUCH;
    header.keylen = Endianconv::intrev16(2);
    header.extlen = (uint8_t)4;
    header.datatype = 0;
    header.reserved = 0;
    header.bodylen = Endianconv::intrev32(6);
    header.opaque = 0;
    header.cas = 0;
    body.expiration = 0;

    buf.append(&req, sizeof(req));
    ASSERT_EQ(DState::AGAIN, codec.decodeRequest(&buf, packet));

    buf.clear();
    buf.append(&req, sizeof(req));
    buf.append("k1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));

    buf.clear();
    header.bodylen = Endianconv::intrev32(2);
    buf.append(&req, sizeof(req) - 4);
    buf.append("k1");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());

    buf.clear();
    header.extlen = (uint8_t)4;
    header.keylen = 0;
    header.bodylen = Endianconv::intrev32(4);
    buf.append(&req, sizeof(req));
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());
}

TEST(MEMCACHED_CODEC_TEST, GET_DECODE_TEST) {
    PacketUniqPtr packet;
    MemcachedCodec codec;
    Buffer buf;

    MemcachedBinReqHeader header;
    header.magic = MEMCACHED_BINARY_REQ_FLAG;
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_GET;
    header.keylen = Endianconv::intrev16(2);
    header.extlen = 0;
    header.datatype = 0;
    header.reserved = 0;
    header.bodylen = Endianconv::intrev32(2);
    header.opaque = 0;
    header.cas = 0;

    buf.append(&header, sizeof(header));
    buf.append("k1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(2U, argv.size());

    buf.clear();
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_GETQ;
    buf.append(&header, sizeof(header));
    buf.append("k1");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    argv.clear();
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(2U, argv.size());

    buf.clear();
    header.keylen = 0;
    buf.append(&header, sizeof(header));
    buf.append("k1");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());
}

TEST(MEMCACHED_CODEC_TEST, AUTH_DECODE_TEST) {
    PacketUniqPtr packet;
    MemcachedCodec codec;
    Buffer buf;

    MemcachedBinReqHeader header;
    header.magic = MEMCACHED_BINARY_REQ_FLAG;
    header.opcode = tair::protocol::MEMCACHED_BINARY_CMD_SASL_AUTH;
    header.keylen = Endianconv::intrev16(5);
    header.extlen = 0;
    header.datatype = 0;
    header.reserved = 0;
    header.bodylen = Endianconv::intrev32(22);
    header.opaque = 0;
    header.cas = 0;

    buf.append(&header, sizeof(header));
    buf.append("PLAINusername:password");
    ASSERT_EQ(DState::SUCCESS, codec.decodeRequest(&buf, packet));
    ASSERT_EQ(0U, buf.length());
    ASSERT_TRUE(packet->instance_of<ArrayPacket>());
    std::vector<std::string> argv;
    packet->packet_cast<ArrayPacket>()->moveBulks(argv);
    ASSERT_EQ(3U, argv.size());

    buf.clear();
    header.keylen = 0;
    buf.append(&header, sizeof(header));
    buf.append("PLAINusername:password");
    ASSERT_EQ(DState::ERROR, codec.decodeRequest(&buf, packet));
    ASSERT_EQ("CLIENT_ERROR bad command line format", codec.getErr());
}