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

#include <string_view>

namespace tair::protocol {

static const constexpr uint8_t MEMCACHED_BINARY_REQ_FLAG = 0x80;
static const constexpr uint8_t MEMCACHED_BINARY_RESP_FLAG = 0x81;

static const constexpr uint8_t MEMCACHED_BINARY_DATA_TYPE_RAW_BYTES = 0x00;

// Definition of the different command opcodes.
enum MemcachedBinaryCommand {
    MEMCACHED_BINARY_CMD_GET = 0x00,             // get
    MEMCACHED_BINARY_CMD_SET = 0x01,             // set
    MEMCACHED_BINARY_CMD_ADD = 0x02,             // add
    MEMCACHED_BINARY_CMD_REPLACE = 0x03,         // replace
    MEMCACHED_BINARY_CMD_DELETE = 0x04,          // delete
    MEMCACHED_BINARY_CMD_INCREMENT = 0x05,       // increment
    MEMCACHED_BINARY_CMD_DECREMENT = 0x06,       // decrement
    MEMCACHED_BINARY_CMD_QUIT = 0x07,            // quit
    MEMCACHED_BINARY_CMD_FLUSH = 0x08,           // flush
    MEMCACHED_BINARY_CMD_GETQ = 0x09,            // getq
    MEMCACHED_BINARY_CMD_NOOP = 0x0a,            // no-op
    MEMCACHED_BINARY_CMD_VERSION = 0x0b,         // version
    MEMCACHED_BINARY_CMD_GETK = 0x0c,            // getk
    MEMCACHED_BINARY_CMD_GETKQ = 0x0d,           // getkq, getk quiet
    MEMCACHED_BINARY_CMD_APPEND = 0x0e,          // append
    MEMCACHED_BINARY_CMD_PREPEND = 0x0f,         // prepend
    MEMCACHED_BINARY_CMD_STAT = 0x10,            // stat (not support this time)
    MEMCACHED_BINARY_CMD_SETQ = 0x11,            // set quiet
    MEMCACHED_BINARY_CMD_ADDQ = 0x12,            // add quiet
    MEMCACHED_BINARY_CMD_REPLACEQ = 0x13,        // replace quiet
    MEMCACHED_BINARY_CMD_DELETEQ = 0x14,         // delete quiet
    MEMCACHED_BINARY_CMD_INCREMENTQ = 0x15,      // increment quiet
    MEMCACHED_BINARY_CMD_DECREMENTQ = 0x16,      // decrement quiet
    MEMCACHED_BINARY_CMD_QUITQ = 0x17,           // quit quiet
    MEMCACHED_BINARY_CMD_FLUSHQ = 0x18,          // flush quiet
    MEMCACHED_BINARY_CMD_APPENDQ = 0x19,         // append quiet
    MEMCACHED_BINARY_CMD_PREPENDQ = 0x1a,        // prepend quiet
    MEMCACHED_BINARY_CMD_TOUCH = 0x1c,           // touch
    MEMCACHED_BINARY_CMD_GAT = 0x1d,             // gat
    MEMCACHED_BINARY_CMD_GATQ = 0x1e,            // gat quiet
    MEMCACHED_BINARY_CMD_SASL_LIST_MECHS = 0x20, // sasl list mechs
    MEMCACHED_BINARY_CMD_SASL_AUTH = 0x21,       // sasl auth
    MEMCACHED_BINARY_CMD_SASL_STEP = 0x22,       // sasl step
    MEMCACHED_BINARY_CMD_CAS = 0x48,             // cas
    MEMCACHED_BINARY_CMD_FAKE = 0xff             // fake command
};

inline bool changeQuietCmd2Cmd(uint8_t &cmd) {
    bool quiet = true;
    switch (cmd) {
        case MEMCACHED_BINARY_CMD_SETQ:
            cmd = MEMCACHED_BINARY_CMD_SET;
            break;
        case MEMCACHED_BINARY_CMD_ADDQ:
            cmd = MEMCACHED_BINARY_CMD_ADD;
            break;
        case MEMCACHED_BINARY_CMD_REPLACEQ:
            cmd = MEMCACHED_BINARY_CMD_REPLACE;
            break;
        case MEMCACHED_BINARY_CMD_DELETEQ:
            cmd = MEMCACHED_BINARY_CMD_DELETE;
            break;
        case MEMCACHED_BINARY_CMD_INCREMENTQ:
            cmd = MEMCACHED_BINARY_CMD_INCREMENT;
            break;
        case MEMCACHED_BINARY_CMD_DECREMENTQ:
            cmd = MEMCACHED_BINARY_CMD_DECREMENT;
            break;
        case MEMCACHED_BINARY_CMD_QUITQ:
            cmd = MEMCACHED_BINARY_CMD_QUIT;
            break;
        case MEMCACHED_BINARY_CMD_FLUSHQ:
            cmd = MEMCACHED_BINARY_CMD_FLUSH;
            break;
        case MEMCACHED_BINARY_CMD_APPENDQ:
            cmd = MEMCACHED_BINARY_CMD_APPEND;
            break;
        case MEMCACHED_BINARY_CMD_PREPENDQ:
            cmd = MEMCACHED_BINARY_CMD_PREPEND;
            break;
        case MEMCACHED_BINARY_CMD_GETQ:
            cmd = MEMCACHED_BINARY_CMD_GET;
            break;
        case MEMCACHED_BINARY_CMD_GETKQ:
            cmd = MEMCACHED_BINARY_CMD_GETK;
            break;
        case MEMCACHED_BINARY_CMD_GATQ:
            cmd = MEMCACHED_BINARY_CMD_GAT;
            break;
        default:
            quiet = false;
    }
    return quiet;
}

static const constexpr std::string_view memcachedCommandTable[35] = {
    {"memcache_get"},
    {"memcache_set"},
    {"memcache_add"},
    {"memcache_replace"},
    {"memcache_delete"},
    {"memcache_incr"},
    {"memcache_decr"},
    {"memcache_quit"},
    {"memcache_flush_all"},
    {"memcache_get"},
    {"memcache_noop"},
    {"memcache_version"},
    {"memcache_getk"},
    {"memcache_getk"},
    {"memcache_append"},
    {"memcache_prepend"},
    {"memcache_stat"},
    {"memcache_set"},
    {"memcache_add"},
    {"memcache_replace"},
    {"memcache_delete"},
    {"memcache_incr"},
    {"memcache_decr"},
    {"memcache_quit"},
    {"memcache_flush_all"},
    {"memcache_append"},
    {"memcache_prepend"},
    {""},
    {"memcache_touch"},
    {"memcache_gat"},
    {"memcache_gat"},
    {""},
    {"memcache_sasl_list_mechs"},
    {"memcache_sasl_auth"},
    {"memcache_sasl_step"}};

enum class MemcachedStatus : uint16_t {
    RESP_SUCCESS = 0x00,
    RESP_KEY_ENOENT = 0x01,
    RESP_KEY_EEXISTS = 0x02,
    RESP_E2BIG = 0x03,
    RESP_EINVAL = 0x04,
    RESP_NOT_STORED = 0x05,
    RESP_DELTA_BADVAL = 0x06,
    RESP_AUTH_ERROR = 0x20,
    RESP_AUTH_CONTINUE = 0x21,
    RESP_UNKNOWN_COMMAND = 0x81,
    RESP_ENOMEM = 0x82,
    RESP_UNKNOWN_ERROR = 0xFF,
};

#pragma pack(1)

// ------------------- request -------------------

// Definition of the memcached binary request header.
struct MemcachedBinReqHeader {
    uint8_t magic;
    uint8_t opcode;
    uint16_t keylen;
    uint8_t extlen;
    uint8_t datatype;
    uint16_t reserved;
    uint32_t bodylen;
    uint32_t opaque;
    uint64_t cas;
};

// Definition of set/add/replace request header.
struct MemcachedBinReqSet {
    MemcachedBinReqHeader header;
    struct {
        uint32_t flags;
        uint32_t expiration;
    } body;
};

// Definition of incr/decr request header.
struct MemcachedBinReqIncr {
    MemcachedBinReqHeader header;
    struct {
        uint64_t delta;
        uint64_t initial;
        uint32_t expiration;
    } body;
};

// Definition of touch request header.
struct MemcachedBinReqTouch {
    MemcachedBinReqHeader header;
    struct {
        uint32_t expiration;
    } body;
};

// Definition of flush request header.
struct MemcachedBinReqFlush {
    MemcachedBinReqHeader header;
    struct {
        uint32_t expiration;
    } body;
};

// ------------------- response -------------------

// Definition of memcache binary response header.
struct MemcachedBinRespHeader {
    uint8_t magic;
    uint8_t opcode;
    uint16_t keylen;
    uint8_t extlen;
    uint8_t datatype;
    uint16_t status;
    uint32_t bodylen;
    uint32_t opaque;
    uint64_t cas;
};

// Definition of status response
struct MemcachedBinRespStatus {
    MemcachedBinRespHeader header;
};

// Definition of get response header.
struct MemcachedBinRespGet {
    MemcachedBinRespHeader header;
    struct {
        uint32_t flags;
    } body;
};

// Definition of incr/decr response header.
struct MemcachedBinRespIncDec {
    MemcachedBinRespHeader header;
    struct {
        uint64_t value;
    } body;
};

#pragma pack()

// Definition of binary context.
struct MemcachedPacketContext {
    uint8_t opcode;
    uint32_t opaque;
    uint64_t cas;
};

} // namespace tair::protocol
