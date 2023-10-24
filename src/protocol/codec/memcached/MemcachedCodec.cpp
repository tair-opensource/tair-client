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
#include "protocol/codec/memcached/MemcachedCodec.hpp"

#include "protocol/ProtocolOptions.hpp"
#include "protocol/packet/resp/ArrayPacket.hpp"

namespace tair::protocol {

using common::StringUtil;

DState MemcachedCodec::decodeRequest(Buffer *buf, PacketUniqPtr &packet) {
    err_.clear();
    if (buf->empty()) {
        return DState::AGAIN;
    }
    uint8_t x = buf->peekInt8();
    if (x == MEMCACHED_BINARY_REQ_FLAG) {
        is_binary_ = true;
        return processBinary(buf, packet);
    } else {
        is_binary_ = false;
        return processText(buf, packet);
    }
}

DState MemcachedCodec::processText(Buffer *buf, PacketUniqPtr &packet) {
    const char *newline = buf->findEOL();
    if (!newline) {
        return DState::AGAIN;
    }
    int linefeed_chars = 1;

    if (newline > buf->data() && *(newline - 1) == '\r') {
        newline--;
        linefeed_chars++;
    }
    std::vector<std::string> argv;
    splitArgs(buf->data(), newline, argv);

    if (argv.empty()) {
        err_ = "ERROR";
        return DState::ERROR;
    }
    if (argv.size() >= 2 && argv[1].size() > ProtocolOptions::PROTO_MEMCACHED_KEY_MAX_LENGTH) {
        err_ = "CLIENT_ERROR bad command line format";
        return DState::ERROR;
    }
    size_t querylen = newline - buf->data();
    size_t datalen = 0;

    if (argv[0] == "set" || argv[0] == "add" || argv[0] == "replace" || argv[0] == "append" || argv[0] == "prepend" || argv[0] == "cas") {
        if (argv.size() < 5) {
            err_ = "CLIENT_ERROR bad command line format";
            return DState::ERROR;
        }
        int bytes;
        if (!SimpleAtoi(argv[4], &bytes)) {
            err_ = "CLIENT_ERROR bad command line format";
            return DState::ERROR;
        }
        if (bytes < 0) {
            err_ = "CLIENT_ERROR bad command line format";
            return DState::ERROR;
        }
        const char *dataline = buf->findEOL(newline + 2);
        if (!dataline) {
            return DState::AGAIN;
        }
        if (dataline - newline - 3 != bytes) {
            err_ = "CLIENT_ERROR bad data chunk";
            return DState::ERROR;
        }

        datalen = dataline - newline - 3;
        if (datalen > ProtocolOptions::memcached_max_item_size) {
            err_ = "SERVER_ERROR object too large for cache";
            return DState::ERROR;
        }
        std::string value = std::string(newline + 2, newline + 2 + bytes);
        argv.emplace_back(value);
    } else if (argv[0] == "mscan") {
        int bytes;
        if (!SimpleAtoi(argv[5], &bytes)) {
            err_ = "CLIENT_ERROR bad command line format";
            return DState::ERROR;
        }
        if (bytes < 0) {
            err_ = "CLIENT_ERROR bad command line format";
            return DState::ERROR;
        }
        const char *dataline = buf->findEOL(newline + 2);
        if (!dataline) {
            return DState::AGAIN;
        }
        if (dataline - newline - 3 != bytes) {
            err_ = "CLIENT_ERROR bad data chunk";
            return DState::ERROR;
        }

        datalen = dataline - newline - 3;
        std::string value = std::string(newline + 2, newline + 2 + bytes);
        argv.emplace_back(value);
    } else if (argv[0] == "incr" || argv[0] == "decr") {
        // init val and expire time
        argv.emplace_back("0");
        argv.emplace_back("0");
    }

    // version
    if (argv[0] == "set" || argv[0] == "add" || argv[0] == "replace" || argv[0] == "append" || argv[0] == "prepend" || argv[0] == "incr" || argv[0] == "decr" || argv[0] == "delete") {
        argv.emplace_back("-1");
    }
    argv[0].insert(0, "memcache_");

    size_t packet_len = querylen + linefeed_chars + datalen;
    buf->skip(packet_len + 2);
    packet = std::make_unique<ArrayPacket>(std::move(argv));
    packet->setPacketSize(packet_len);

    return DState::SUCCESS;
}

DState MemcachedCodec::processBinary(Buffer *buf, PacketUniqPtr &packet) {
    static constexpr size_t HEADER_LEN = sizeof(MemcachedBinReqHeader);
    if (buf->size() < HEADER_LEN) {
        return DState::AGAIN;
    }

    MemcachedBinReqHeader *req = (MemcachedBinReqHeader *)(buf->data());
    uint16_t keylen = ntohu16(req->keylen);
    uint16_t extlen = (uint8_t)(req->extlen);
    uint32_t bodylen = ntohu32(req->bodylen);

    if (keylen > ProtocolOptions::PROTO_MEMCACHED_KEY_MAX_LENGTH) {
        err_ = "CLIENT_ERROR bad command line format";
        return DState::ERROR;
    }

    if (buf->size() < HEADER_LEN + bodylen) {
        return DState::AGAIN;
    }

    uint8_t opcode = req->opcode;
    uint64_t cas = ntohu64(req->cas);
    uint32_t opaque = req->opaque;

    MemcachedPacketContext context;
    context.opcode = opcode;
    context.opaque = opaque;
    context.cas = cas;
    context_ = std::any(context);

    std::vector<std::string> argv;
    bool quiet = changeQuietCmd2Cmd(opcode);

    bool protocol_error = false;
    switch (opcode) {
        case MEMCACHED_BINARY_CMD_VERSION:
        case MEMCACHED_BINARY_CMD_QUIT:
        case MEMCACHED_BINARY_CMD_NOOP:
        case MEMCACHED_BINARY_CMD_SASL_LIST_MECHS:
            if (extlen == 0 && keylen == 0 && bodylen == 0) {
                argv.emplace_back(memcachedCommandTable[opcode]);

                buf->skip(HEADER_LEN);
            } else {
                protocol_error = true;
            }
            break;
        case MEMCACHED_BINARY_CMD_FLUSH:
            if (extlen == 4 && keylen == 0 && bodylen == 4) {
                MemcachedBinReqFlush *header = (MemcachedBinReqFlush *)(buf->data());
                auto expire = ntohu32(header->body.expiration);

                argv.emplace_back(memcachedCommandTable[opcode]);
                if (expire != 0) {
                    argv.emplace_back(std::to_string(expire));
                }

                buf->skip(HEADER_LEN + bodylen);
            } else {
                protocol_error = true;
            }
            break;
        case MEMCACHED_BINARY_CMD_SET:
        case MEMCACHED_BINARY_CMD_ADD:
        case MEMCACHED_BINARY_CMD_REPLACE:
            if (extlen == 8 && keylen != 0) {
                if (bodylen - keylen - extlen > ProtocolOptions::memcached_max_item_size) {
                    err_ = "SERVER_ERROR object too large for cache";
                    return DState::ERROR;
                }
                MemcachedBinReqSet *header = (MemcachedBinReqSet *)(buf->data());
                std::string key(buf->data() + HEADER_LEN + extlen, keylen);
                std::string flag = std::to_string(htonu32(header->body.flags));
                std::string expire = std::to_string(htonu32(header->body.expiration));
                std::string value(buf->data() + HEADER_LEN + extlen + keylen, bodylen - keylen - extlen);

                argv.emplace_back(memcachedCommandTable[opcode]);
                argv.emplace_back(key);
                argv.emplace_back(flag);
                argv.emplace_back(expire);
                argv.emplace_back(std::to_string(bodylen - keylen - extlen));
                argv.emplace_back(value);
                argv.emplace_back(std::to_string(cas));

                buf->skip(HEADER_LEN + bodylen);
            } else {
                protocol_error = true;
            }
            break;
        case MEMCACHED_BINARY_CMD_APPEND:
        case MEMCACHED_BINARY_CMD_PREPEND:
            if (keylen > 0 && extlen == 0) {
                std::string key(buf->data() + HEADER_LEN, keylen);
                std::string value(buf->data() + HEADER_LEN + keylen, bodylen - keylen);

                argv.emplace_back(memcachedCommandTable[opcode]);
                argv.emplace_back(key);
                argv.emplace_back("0");
                argv.emplace_back("0");
                argv.emplace_back(std::to_string(bodylen - keylen - extlen));
                argv.emplace_back(value);
                argv.emplace_back(std::to_string(cas));

                buf->skip(HEADER_LEN + bodylen);
            } else {
                protocol_error = true;
            }
            break;
        case MEMCACHED_BINARY_CMD_DELETE:
            if (extlen == 0 && bodylen == keylen && keylen > 0) {
                std::string key(buf->data() + HEADER_LEN, keylen);

                argv.emplace_back(memcachedCommandTable[opcode]);
                argv.emplace_back(key);
                argv.emplace_back(std::to_string(cas));

                buf->skip(HEADER_LEN + bodylen);
            } else {
                protocol_error = true;
            }
            break;
        case MEMCACHED_BINARY_CMD_GET:
        case MEMCACHED_BINARY_CMD_GETK:
            if (extlen == 0 && bodylen == keylen && keylen > 0) {
                std::string key(buf->data() + HEADER_LEN, keylen);

                argv.emplace_back(memcachedCommandTable[opcode]);
                argv.emplace_back(key);

                buf->skip(HEADER_LEN + bodylen);
            } else {
                protocol_error = true;
            }
            break;
        case MEMCACHED_BINARY_CMD_INCREMENT:
        case MEMCACHED_BINARY_CMD_DECREMENT:
            if (keylen > 0 && extlen == 20 && bodylen == (keylen + extlen)) {
                MemcachedBinReqIncr *header = (MemcachedBinReqIncr *)(buf->data());
                std::string key(buf->data() + HEADER_LEN + extlen, keylen);
                std::string delta = std::to_string(ntohu64(header->body.delta));
                std::string inital = std::to_string(ntohu64(header->body.initial));
                std::string expire = std::to_string(ntohu32(header->body.expiration));

                argv.emplace_back(memcachedCommandTable[opcode]);
                argv.emplace_back(key);
                argv.emplace_back(delta);
                argv.emplace_back(inital);
                argv.emplace_back(expire);
                argv.emplace_back(std::to_string(cas));

                buf->skip(HEADER_LEN + bodylen);
            } else {
                protocol_error = true;
            }
            break;
        case MEMCACHED_BINARY_CMD_SASL_AUTH:
        case MEMCACHED_BINARY_CMD_SASL_STEP:
            if (extlen == 0 && keylen != 0) {
                std::string key(buf->data() + HEADER_LEN, keylen);
                std::string value(buf->data() + HEADER_LEN + keylen, bodylen - keylen);

                argv.emplace_back(memcachedCommandTable[opcode]);
                argv.emplace_back(key);
                argv.emplace_back(value);

                buf->skip(HEADER_LEN + bodylen);
            } else {
                protocol_error = true;
            }
            break;
        case MEMCACHED_BINARY_CMD_TOUCH:
        case MEMCACHED_BINARY_CMD_GAT:
            if (extlen == 4 && keylen > 0 && extlen + keylen == bodylen) {
                MemcachedBinReqTouch *header = (MemcachedBinReqTouch *)(buf->data());
                std::string key(buf->data() + HEADER_LEN + extlen, keylen);
                std::string expire = std::to_string(ntohu32(header->body.expiration));

                argv.emplace_back(memcachedCommandTable[opcode]);
                argv.emplace_back(key);
                argv.emplace_back(expire);

                buf->skip(HEADER_LEN + bodylen);
            } else {
                protocol_error = true;
            }
            break;
        default:
            err_ = "Unknown Command";
            return DState::ERROR;
    }

    if (protocol_error) {
        err_ = "CLIENT_ERROR bad command line format";
        return DState::ERROR;
    }

    packet = std::make_unique<ArrayPacket>(std::move(argv));
    packet->setPacketSize(HEADER_LEN + bodylen);

    ArrayPacket *array_packet = packet->packet_cast<ArrayPacket>();
    array_packet->setContext(context);
    array_packet->setQuiet(quiet);

    return DState::SUCCESS;
}

DState MemcachedCodec::encodeResponse(Buffer *buf, Packet *packet) {
    if (is_binary_) {
        return packet->encodeMemcachedBinary(buf);
    } else {
        return packet->encodeMemcachedText(buf);
    }
}

DState MemcachedCodec::encodeRequest(Buffer *buf, Packet *packet) {
    return DState::ERROR;
}

DState MemcachedCodec::decodeResponse(Buffer *buf, PacketUniqPtr &packet) {
    return DState::ERROR;
}

} // namespace tair::protocol
