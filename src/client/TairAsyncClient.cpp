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
#include "client/TairAsyncClient.hpp"

#include "protocol/packet/resp/ArrayPacket.hpp"
#include "client/TairResultHelper.hpp"

namespace tair::client {

using protocol::SimpleStringPacket;
using protocol::BulkStringPacket;
using protocol::IntegerPacket;
using protocol::ArrayPacket;

TairResult<std::string> TairAsyncClient::init() {
    return TairBaseClient::connect().get();
}

void TairAsyncClient::destroy() {
    TairBaseClient::disconnect();
}

void TairAsyncClient::setServerAddr(const std::string &addr) {
    TairBaseClient::setServerAddr(addr);
}

const std::string &TairAsyncClient::getServerAddr() const {
    return TairBaseClient::getServerAddr();
}

void TairAsyncClient::setUser(const std::string &user) {
    TairBaseClient::setUser(user);
}

void TairAsyncClient::setPassword(const std::string &password) {
    TairBaseClient::setPassword(password);
}

void TairAsyncClient::setConnectingTimeoutMs(int timeout_ms) {
    TairBaseClient::setConnectingTimeoutMs(timeout_ms);
}

void TairAsyncClient::setReconnectIntervalMs(int timeout_ms) {
    TairBaseClient::setReconnectIntervalMs(timeout_ms);
}

void TairAsyncClient::setAutoReconnect(bool reconnect) {
    TairBaseClient::setAutoReconnect(reconnect);
}

void TairAsyncClient::setKeepAliveSeconds(int seconds) {
    TairBaseClient::setKeepAliveSeconds(seconds);
}

// -------------------------------- send Command --------------------------------
void TairAsyncClient::sendCommand(CommandArgv &&argv, const ResultPacketCallback &callback) {
    TairBaseClient::sendCommand(std::move(argv), [this, callback](auto &req, auto &resp, int64_t) {
        callback(this, req, resp);
    });
}

void TairAsyncClient::sendCommand(const CommandArgv &argv, const ResultPacketCallback &callback) {
    TairBaseClient::sendCommand(argv, [this, callback](auto &req, auto &resp, int64_t) {
        callback(this, req, resp);
    });
}

void TairAsyncClient::sendCommand(CommandArgv &&argv, const ResultPacketAndLatencyCallback &callback) {
    TairBaseClient::sendCommand(std::move(argv), [this, callback](auto &req, auto &resp, int64_t latency_us) {
        callback(this, req, resp, latency_us);
    });
}

void TairAsyncClient::sendCommand(const CommandArgv &argv, const ResultPacketAndLatencyCallback &callback) {
    TairBaseClient::sendCommand(argv, [this, callback](auto &req, auto &resp, int64_t latency_us) {
        callback(this, req, resp, latency_us);
    });
}

// -------------------------------- Generic Command --------------------------------
void TairAsyncClient::del(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"del", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::del(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"del"};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::unlink(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"unlink", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::unlink(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"unlink"};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::exists(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"exists", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::exists(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"exists"};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::expire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback) {
    sendCommand({"expire", key, std::to_string(timeout)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::expire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"expire", key, std::to_string(timeout)};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::expireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback) {
    sendCommand({"expireat", key, std::to_string(timestamp)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::expireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"expireat", key, std::to_string(timestamp)};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::persist(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"persist", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::pexpire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback) {
    sendCommand({"pexpire", key, std::to_string(timeout)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::pexpire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"pexpire", key, std::to_string(timeout)};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::pexpireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback) {
    sendCommand({"pexpireat", key, std::to_string(timestamp)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::pexpireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"pexpireat", key, std::to_string(timestamp)};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::ttl(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"ttl", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::pttl(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"pttl", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::touch(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"touch"};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::dump(const std::string &key, const ResultStringPtrCallback &callback) {
    sendCommand({"dump", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::restore(const std::string &key, int64_t ttl, const std::string &value, const ResultStringCallback &callback) {
    sendCommand({"restore", key, std::to_string(ttl), value}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::restore(const std::string &key, int64_t ttl, const std::string &value, const RestoreParams &params, const ResultStringCallback &callback) {
    CommandArgv argv {"restore", key, std::to_string(ttl), value};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::keys(const std::string &pattern, const ResultVectorStringCallback &callback) {
    sendCommand({"keys", pattern}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::move(const std::string &key, int64_t db, const ResultIntegerCallback &callback) {
    sendCommand({"move", key, std::to_string(db)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::randomkey(const ResultStringPtrCallback &callback) {
    sendCommand({"randomkey"}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::rename(const std::string &key, const std::string &newkey, const ResultStringCallback &callback) {
    sendCommand({"rename", key, newkey}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::renamenx(const std::string &key, const std::string &newkey, const ResultIntegerCallback &callback) {
    sendCommand({"renamenx", key, newkey}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::type(const std::string &key, const ResultStringCallback &callback) {
    sendCommand({"type", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::scan(const std::string &cursor, const ResultScanCallback &callback) {
    sendCommand({"scan", cursor}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, ScanResult>(resp, TairResultHelper::scanResultBuilder, callback);
    });
}

void TairAsyncClient::scan(const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    CommandArgv argv {"scan", cursor};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, ScanResult>(resp, TairResultHelper::scanResultBuilder, callback);
    });
}

void TairAsyncClient::sort(const std::string &key, const ResultVectorStringPtrCallback &callback) {
    sendCommand({"sort", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::shared_ptr<std::string>>>(resp, TairResultHelper::vectorStringPtrBuilder, callback);
    });
}

void TairAsyncClient::sort(const std::string &key, const SortParams &params, const ResultVectorStringPtrCallback &callback) {
    CommandArgv argv {"sort", key};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::shared_ptr<std::string>>>(resp, TairResultHelper::vectorStringPtrBuilder, callback);
    });
}

void TairAsyncClient::sortStore(const std::string &key, const std::string &storekey, const ResultIntegerCallback &callback) {
    sendCommand({"sort", key, "store", storekey}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::sortStore(const std::string &key, const std::string &storekey, const SortParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"sort", key, "store", storekey};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::copy(const std::string &key, const std::string &destkey, const ResultIntegerCallback &callback) {
    sendCommand({"copy", key, destkey}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::copy(const std::string &key, const std::string &destkey, const CopyParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"copy", key, destkey};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

// -------------------------------- String Command --------------------------------
void TairAsyncClient::append(const std::string &key, const std::string &value, const ResultIntegerCallback &callback) {
    sendCommand({"append", key, value}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::bitcount(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"bitcount", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::bitcount(const std::string &key, const BitPositonParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"bitcount", key};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::bitfield(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback) {
    CommandArgv argv {"bitfield", key};
    argv.insert(argv.end(), args);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::shared_ptr<int64_t>>>(resp, TairResultHelper::vectorIntegerPtrBuilder, callback);
    });
}

void TairAsyncClient::bitfieldRo(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback) {
    CommandArgv argv {"bitfield_ro", key};
    argv.insert(argv.end(), args);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::shared_ptr<int64_t>>>(resp, TairResultHelper::vectorIntegerPtrBuilder, callback);
    });
}

void TairAsyncClient::bitop(const BitOperation &op, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"bitop", bitOperationToString(op)};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::bitpos(const std::string &key, int64_t bit, const ResultIntegerCallback &callback) {
    sendCommand({"bitpos", key, std::to_string(bit)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::bitpos(const std::string &key, int64_t bit, const BitPositonParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"bitpos", key, std::to_string(bit)};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::setbit(const std::string &key, int64_t offset, int64_t value, const ResultIntegerCallback &callback) {
    sendCommand({"setbit", key, std::to_string(offset), std::to_string(value)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::getbit(const std::string &key, int64_t offset, const ResultIntegerCallback &callback) {
    sendCommand({"getbit", key, std::to_string(offset)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::decr(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"decr", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::decrby(const std::string &key, int64_t decrement, const ResultIntegerCallback &callback) {
    sendCommand({"decrby", key, std::to_string(decrement)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::getrange(const std::string &key, int64_t start, int64_t end, const ResultStringCallback &callback) {
    sendCommand({"getrange", key, std::to_string(start), std::to_string(end)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<BulkStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::getset(const std::string &key, const std::string &value, const ResultStringPtrCallback &callback) {
    sendCommand({"getset", key, value}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::set(const std::string &key, const std::string &value, const SetParams &params, const ResultStringCallback &callback) {
    CommandArgv argv {"set", key, value};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::set(const std::string &key, const std::string &value, const ResultStringCallback &callback) {
    sendCommand({"set", key, value}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::get(const std::string &key, const ResultStringPtrCallback &callback) {
    sendCommand({"get", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::incr(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"incr", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::incrby(const std::string &key, int64_t increment, const ResultIntegerCallback &callback) {
    sendCommand({"incrby", key, std::to_string(increment)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::incrbyfloat(const std::string &key, double increment, const ResultStringCallback &callback) {
    sendCommand({"incrbyfloat", key, std::to_string(increment)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<BulkStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::mget(InitializerList<std::string> keys, const ResultVectorStringPtrCallback &callback) {
    CommandArgv argv {"mget"};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::shared_ptr<std::string>>>(resp, TairResultHelper::vectorStringPtrBuilder, callback);
    });
}

void TairAsyncClient::mset(InitializerList<std::string> kvs, const ResultStringCallback &callback) {
    CommandArgv argv {"mset"};
    argv.insert(argv.end(), kvs);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::msetnx(InitializerList<std::string> kvs, const ResultIntegerCallback &callback) {
    CommandArgv argv {"msetnx"};
    argv.insert(argv.end(), kvs);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::psetex(const std::string &key, int64_t milliseconds, const std::string &value, const ResultStringCallback &callback) {
    sendCommand({"psetex", key, std::to_string(milliseconds), value}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::setex(const std::string &key, int64_t seconds, const std::string &value, const ResultStringCallback &callback) {
    sendCommand({"setex", key, std::to_string(seconds), value}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::setnx(const std::string &key, const std::string &value, const ResultIntegerCallback &callback) {
    sendCommand({"setnx", key, value}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::setrange(const std::string &key, int64_t offset, const std::string &value, const ResultIntegerCallback &callback) {
    sendCommand({"setrange", key, std::to_string(offset), value}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::strlen(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"strlen", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::getdel(const std::string &key, const ResultStringPtrCallback &callback) {
    sendCommand({"getdel", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::getex(const std::string &key, const GetExParams &params, const ResultStringPtrCallback &callback) {
    CommandArgv argv {"getex", key};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

// -------------------------------- List Command --------------------------------
void TairAsyncClient::blpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"blpop"};
    argv.insert(argv.end(), keys);
    argv.emplace_back(std::to_string(timeout));

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::brpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"brpop"};
    argv.insert(argv.end(), keys);
    argv.emplace_back(std::to_string(timeout));

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::brpoplpush(const std::string &src, const std::string &dest, int64_t timeout, const ResultStringPtrCallback &callback) {
    sendCommand({"brpoplpush", src, dest, std::to_string(timeout)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::lindex(const std::string &key, int64_t index, const ResultStringPtrCallback &callback) {
    sendCommand({"lindex", key, std::to_string(index)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::linsert(const std::string &key, const ListDirection &direction, const std::string &pivot, const std::string &element, const ResultIntegerCallback &callback) {
    sendCommand({"linsert", key, listDirectionToString(direction), pivot, element}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::llen(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"llen", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::lpop(const std::string &key, const ResultStringPtrCallback &callback) {
    sendCommand({"lpop", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::lpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    sendCommand({"lpop", key, std::to_string(count)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::lpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    CommandArgv argv {"lpush", key};
    argv.insert(argv.end(), elements);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::lpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    CommandArgv argv {"lpushx", key};
    argv.insert(argv.end(), elements);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::lrange(const std::string &key, int64_t start, int64_t stop, const ResultVectorStringCallback &callback) {
    sendCommand({"lrange", key, std::to_string(start), std::to_string(stop)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::lrem(const std::string &key, int64_t count, const std::string &element, const ResultIntegerCallback &callback) {
    sendCommand({"lrem", key, std::to_string(count), element}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::lset(const std::string &key, int64_t index, const std::string &element, const ResultStringCallback &callback) {
    sendCommand({"lset", key, std::to_string(index), element}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::ltrim(const std::string &key, int64_t start, int64_t stop, const ResultStringCallback &callback) {
    sendCommand({"ltrim", key, std::to_string(start), std::to_string(stop)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::rpop(const std::string &key, const ResultStringPtrCallback &callback) {
    sendCommand({"rpop", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}
void TairAsyncClient::rpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    sendCommand({"rpop", key, std::to_string(count)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::rpoplpush(const std::string &src, const std::string &dest, const ResultStringPtrCallback &callback) {
    sendCommand({"rpoplpush", src, dest}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::rpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    CommandArgv argv {"rpush", key};
    argv.insert(argv.end(), elements);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::rpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    CommandArgv argv {"rpushx", key};
    argv.insert(argv.end(), elements);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::lmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, const ResultStringPtrCallback &callback) {
    sendCommand({"lmove", src, dest, listDirectionToString(ld), listDirectionToString(rd)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::blmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, int64_t timeout, const ResultStringPtrCallback &callback) {
    sendCommand({"blmove", src, dest, listDirectionToString(ld), listDirectionToString(rd), std::to_string(timeout)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

// -------------------------------- Set Command --------------------------------
void TairAsyncClient::sadd(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) {
    CommandArgv argv {"sadd", key};
    argv.insert(argv.end(), members);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::scard(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"scard", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::sismember(const std::string &key, const std::string &member, const ResultIntegerCallback &callback) {
    sendCommand({"sismember", key, member}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::smembers(const std::string &key, const ResultVectorStringCallback &callback) {
    sendCommand({"smembers", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::smove(const std::string &src, const std::string &dest, const std::string &member, const ResultIntegerCallback &callback) {
    sendCommand({"smove", src, dest, member}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::srem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) {
    CommandArgv argv {"srem", key};
    argv.insert(argv.end(), members);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::spop(const std::string &key, const ResultStringPtrCallback &callback) {
    sendCommand({"spop", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::spop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    sendCommand({"spop", key, std::to_string(count)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::srandmember(const std::string &key, const ResultStringPtrCallback &callback) {
    sendCommand({"srandmember", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::srandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    sendCommand({"srandmember", key, std::to_string(count)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::sdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"sdiff"};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::sinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"sinter"};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::sunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"sunion"};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::sdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"sdiffstore", dest};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::sinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"sinterstore", dest};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::sunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"sunionstore", dest};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::sscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) {
    sendCommand({"sscan", key, cursor}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, ScanResult>(resp, TairResultHelper::scanResultBuilder, callback);
    });
}

void TairAsyncClient::sscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    CommandArgv argv {"sscan", key, cursor};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, ScanResult>(resp, TairResultHelper::scanResultBuilder, callback);
    });
}

void TairAsyncClient::smismember(const std::string &key, InitializerList<std::string> members, const ResultVectorIntegerCallback &callback) {
    CommandArgv argv {"smismember", key};
    argv.insert(argv.end(), members);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<int64_t>>(resp, TairResultHelper::vectorIntegerBuilder, callback);
    });
}

// -------------------------------- Hash Command --------------------------------
void TairAsyncClient::hdel(const std::string &key, InitializerList<std::string> fields, const ResultIntegerCallback &callback) {
    CommandArgv argv {"hdel", key};
    argv.insert(argv.end(), fields);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::hexists(const std::string &key, const std::string &field, const ResultIntegerCallback &callback) {
    sendCommand({"hexists", key, field}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::hget(const std::string &key, const std::string &field, const ResultStringPtrCallback &callback) {
    sendCommand({"hget", key, field}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::hgetall(const std::string &key, const ResultVectorStringCallback &callback) {
    sendCommand({"hgetall", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::hincrby(const std::string &key, const std::string &field, int64_t increment, const ResultIntegerCallback &callback) {
    sendCommand({"hincrby", key, field, std::to_string(increment)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::hincrbyfloat(const std::string &key, const std::string &field, double increment, const ResultStringCallback &callback) {
    sendCommand({"hincrbyfloat", key, field, std::to_string(increment)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<BulkStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::hkeys(const std::string &key, const ResultVectorStringCallback &callback) {
    sendCommand({"hkeys", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::hvals(const std::string &key, const ResultVectorStringCallback &callback) {
    sendCommand({"hvals", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::hlen(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"hlen", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::hmget(const std::string &key, InitializerList<std::string> fields, const ResultVectorStringPtrCallback &callback) {
    CommandArgv argv {"hmget", key};
    argv.insert(argv.end(), fields);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::shared_ptr<std::string>>>(resp, TairResultHelper::vectorStringPtrBuilder, callback);
    });
}

void TairAsyncClient::hset(const std::string &key, const std::string &filed, const std::string &value, const ResultIntegerCallback &callback) {
    sendCommand({"hset", key, filed, value}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::hset(const std::string &key, InitializerList<std::string> kvs, const ResultIntegerCallback &callback) {
    CommandArgv argv {"hset", key};
    argv.insert(argv.end(), kvs);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::hsetnx(const std::string &key, const std::string &field, const std::string &value, const ResultIntegerCallback &callback) {
    sendCommand({"hsetnx", key, field, value}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::hstrlen(const std::string &key, const std::string &field, const ResultIntegerCallback &callback) {
    sendCommand({"hstrlen", key, field}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::hscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) {
    sendCommand({"hscan", key, cursor}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, ScanResult>(resp, TairResultHelper::scanResultBuilder, callback);
    });
}

void TairAsyncClient::hscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    CommandArgv argv {"hscan", key, cursor};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, ScanResult>(resp, TairResultHelper::scanResultBuilder, callback);
    });
}

void TairAsyncClient::hrandfield(const std::string &key, const ResultStringPtrCallback &callback) {
    sendCommand({"hrandfield", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::hrandfield(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    sendCommand({"hrandfield", key, std::to_string(count)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

// -------------------------------- Zset Command --------------------------------
void TairAsyncClient::zadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    CommandArgv argv {"zadd", key};
    argv.insert(argv.end(), elements);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zadd(const std::string &key, const ZAddParams &params, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    CommandArgv argv {"zadd", key};
    params.addParamsToArgv(argv);
    argv.insert(argv.end(), elements);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zincrby(const std::string &key, int64_t increment, const std::string &member, const ResultStringCallback &callback) {
    sendCommand({"zincrby", key, std::to_string(increment), member}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<BulkStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::zscore(const std::string &key, const std::string &member, const ResultStringPtrCallback &callback) {
    sendCommand({"zscore", key, member}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::zrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback) {
    sendCommand({"zrank", key, member}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doIntegerPtrCallback(resp, callback);
    });
}

void TairAsyncClient::zrevrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback) {
    sendCommand({"zrevrank", key, member}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doIntegerPtrCallback(resp, callback);
    });
}

void TairAsyncClient::zcard(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"zcard", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) {
    sendCommand({"zcount", key, min, max}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zlexcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) {
    sendCommand({"zlexcount", key, min, max}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zpopmax(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    sendCommand({"zpopmax", key, std::to_string(count)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zpopmin(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    sendCommand({"zpopmin", key, std::to_string(count)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::bzpopmax(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"bzpopmax"};
    argv.insert(argv.end(), keys);
    argv.emplace_back(std::to_string(timeout));

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::bzpopmin(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"bzpopmin"};
    argv.insert(argv.end(), keys);
    argv.emplace_back(std::to_string(timeout));

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zrange(const std::string &key, const std::string &start, const std::string &stop, const ResultVectorStringCallback &callback) {
    sendCommand({"zrange", key, start, stop}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zrange(const std::string &key, const std::string &start, const std::string &stop, const ZRangeParams &params, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"zrange", key, start, stop};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) {
    sendCommand({"zrangestore", dest, src, min, max}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ZRangeParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"zrangestore", dest, src, min, max};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zrem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) {
    CommandArgv argv {"zrem", key};
    argv.insert(argv.end(), members);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zremrange(const ZRemRangeOption &option, const std::string &key, const std::string &begin, const std::string &end, const ResultIntegerCallback &callback) {
    sendCommand({"zremrange" + zremRangeOptionToString(option), key, begin, end}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"zinter", std::to_string(keys.size())};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zinter(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"zinter", std::to_string(keys.size())};
    argv.insert(argv.end(), keys);
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"zinterstore", dest, std::to_string(keys.size())};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zinterstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"zinterstore", dest, std::to_string(keys.size())};
    argv.insert(argv.end(), keys);
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"zunion", std::to_string(keys.size())};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zunion(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"zunion", std::to_string(keys.size())};
    argv.insert(argv.end(), keys);
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"zunionstore", dest, std::to_string(keys.size())};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zunionstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback) {
    CommandArgv argv {"zunionstore", dest, std::to_string(keys.size())};
    argv.insert(argv.end(), keys);
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"zdiff", std::to_string(keys.size())};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zdiffWithScores(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    CommandArgv argv {"zdiff", std::to_string(keys.size())};
    argv.insert(argv.end(), keys);
    argv.emplace_back("withscores");

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"zdiffstore", dest, std::to_string(keys.size())};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zdiffstoreWithScores(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"zdiffstore", dest, std::to_string(keys.size())};
    argv.insert(argv.end(), keys);
    argv.emplace_back("withscores");

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::zmscore(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback) {
    CommandArgv argv {"zmscore", key};
    argv.insert(argv.end(), members);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::shared_ptr<std::string>>>(resp, TairResultHelper::vectorStringPtrBuilder, callback);
    });
}

void TairAsyncClient::zrandmember(const std::string &key, const ResultStringPtrCallback &callback) {
    sendCommand({"zrandmember", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}

void TairAsyncClient::zrandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    sendCommand({"zrandmember", key, std::to_string(count)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::zscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) {
    sendCommand({"zscan", key, cursor}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, ScanResult>(resp, TairResultHelper::scanResultBuilder, callback);
    });
}

void TairAsyncClient::zscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    CommandArgv argv {"zscan", key, cursor};
    params.addParamsToArgv(argv);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, ScanResult>(resp, TairResultHelper::scanResultBuilder, callback);
    });
}

// -------------------------------- HyperLogLog Command --------------------------------
void TairAsyncClient::pfadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    CommandArgv argv {"pfadd", key};
    argv.insert(argv.end(), elements);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::pfcount(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    CommandArgv argv {"pfcount"};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::pfmerge(const std::string &dest, InitializerList<std::string> keys, const ResultStringCallback &callback) {
    CommandArgv argv {"pfmerge", dest};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

// -------------------------------- Geo Command --------------------------------
void TairAsyncClient::geoadd(const std::string &key, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback) {
    CommandArgv argv {"geoadd", key};
    for (auto &member : members) {
        argv.emplace_back(std::to_string(std::get<0>(member)));
        argv.emplace_back(std::to_string(std::get<1>(member)));
        argv.emplace_back(std::get<2>(member));
    }

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::geoadd(const std::string &key, const GeoAddParams &params, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback) {
    CommandArgv argv {"geoadd", key};
    params.addParamsToArgv(argv);
    for (auto &member : members) {
        argv.emplace_back(std::to_string(std::get<0>(member)));
        argv.emplace_back(std::to_string(std::get<1>(member)));
        argv.emplace_back(std::get<2>(member));
    }

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::geodist(const std::string &key, const std::string &member1, const std::string &member2, const GeoUnit &unit, const ResultStringPtrCallback &callback) {
    sendCommand({"geodist", key, member1, member2, geoUnitToString(unit)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<BulkStringPacket, std::shared_ptr<std::string>>(resp, TairResultHelper::stringPtrBuilder, callback);
    });
}
void TairAsyncClient::geohash(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback) {
    CommandArgv argv {"geohash", key};
    argv.insert(argv.end(), members);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::shared_ptr<std::string>>>(resp, TairResultHelper::vectorStringPtrBuilder, callback);
    });
}

void TairAsyncClient::geopos(const std::string &key, InitializerList<std::string> members, const ResultGeoposCallback &callback) {
    CommandArgv argv {"geopos", key};
    argv.insert(argv.end(), members);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, GeoPosResult>(resp, TairResultHelper::geoposResultBuilder, callback);
    });
}

void TairAsyncClient::georadius(const std::string &key, double longitude, double latitude, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback) {
    sendCommand({"georadius", key, std::to_string(longitude), std::to_string(latitude), std::to_string(radius), geoUnitToString(unit)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

void TairAsyncClient::georadiusbymember(const std::string &key, const std::string &member, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback) {
    sendCommand({"georadiusbymember", key, member, std::to_string(radius), geoUnitToString(unit)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<std::string>>(resp, TairResultHelper::vectorStringBuilder, callback);
    });
}

// -------------------------------- Stream Command --------------------------------
void TairAsyncClient::xadd(const std::string &key, const std::string &id, InitializerList<std::string> elements, const ResultStringCallback &callback) {
    CommandArgv argv {"xadd", key, id};
    argv.insert(argv.end(), elements);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<BulkStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::xlen(const std::string &key, const ResultIntegerCallback &callback) {
    sendCommand({"xlen", key}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::xdel(const std::string &key, InitializerList<std::string> ids, const ResultIntegerCallback &callback) {
    CommandArgv argv {"xdel", key};
    argv.insert(argv.end(), ids);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::xack(const std::string &key, const std::string &group, InitializerList<std::string> ids, const ResultIntegerCallback &callback) {
    CommandArgv argv {"xack", key, group};
    argv.insert(argv.end(), ids);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::xgroupCreate(const std::string &key, const std::string &group, const std::string &id, const ResultStringCallback &callback) {
    sendCommand({"xgroup", "create", key, group, id}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::xgroupCreateConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback) {
    sendCommand({"xgroup", "createconsumer", key, group, consumer}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::xgroupDelConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback) {
    sendCommand({"xgroup", "delconsumer", key, group, consumer}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::xgroupDestroy(const std::string &key, const std::string &group, const ResultIntegerCallback &callback) {
    sendCommand({"xgroup", "destroy", key, group}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::xgroupSetID(const std::string &key, const std::string &group, const ResultStringCallback &callback) {
    sendCommand({"xgroup", "setid", key, group}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::xpending(const std::string &key, const std::string &group, const ResultXPendingCallback &callback) {
    sendCommand({"xpending", key, group}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, XPendingResult>(resp, TairResultHelper::xpendingResultBuilder, callback);
    });
}

void TairAsyncClient::xrange(const std::string &key, const std::string &start, const std::string &end, const ResultXRangeCallback &callback) {
    sendCommand({"xrange", key, start, end}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<XRangeResult>>(resp, TairResultHelper::xrangeResultBuilder, callback);
    });
}

void TairAsyncClient::xrevrange(const std::string &key, const std::string &end, const std::string &start, const ResultXRangeCallback &callback) {
    sendCommand({"xrevrange", key, end, start}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<XRangeResult>>(resp, TairResultHelper::xrangeResultBuilder, callback);
    });
}

void TairAsyncClient::xread(int64_t count, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback) {
    CommandArgv argv {"xread", "count", std::to_string(count), "streams"};
    argv.insert(argv.end(), keys);
    argv.insert(argv.end(), ids);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<XReadResult>>(resp, TairResultHelper::xreadResultBuilder, callback);
    });
}

void TairAsyncClient::xreadgroup(const std::string &group, const std::string &consumer, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback) {
    CommandArgv argv {"xreadgroup", "group", group, consumer, "streams"};
    argv.insert(argv.end(), keys);
    argv.insert(argv.end(), ids);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<XReadResult>>(resp, TairResultHelper::xreadResultBuilder, callback);
    });
}

void TairAsyncClient::xtrim(const std::string &key, const std::string &strategy, int64_t threshold, const ResultIntegerCallback &callback) {
    sendCommand({"xtrim", key, strategy, std::to_string(threshold)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::xsetid(const std::string &key, const std::string &last_id, const ResultStringCallback &callback) {
    sendCommand({"xsetid", key, last_id}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

// -------------------------------- Script Command --------------------------------
void TairAsyncClient::eval(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    CommandArgv argv {"eval", script, std::to_string(keys.size())};
    argv.insert(argv.end(), keys);
    argv.insert(argv.end(), args);

    sendCommand(argv, callback);
}

void TairAsyncClient::evalRo(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    CommandArgv argv {"eval_ro", script, std::to_string(keys.size())};
    argv.insert(argv.end(), keys);
    argv.insert(argv.end(), args);

    sendCommand(argv, callback);
}

void TairAsyncClient::evalsha(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    CommandArgv argv {"evalsha", sha1, std::to_string(keys.size())};
    argv.insert(argv.end(), keys);
    argv.insert(argv.end(), args);

    sendCommand(argv, callback);
}

void TairAsyncClient::evalshaRo(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    CommandArgv argv {"evalsha_ro", sha1, std::to_string(keys.size())};
    argv.insert(argv.end(), keys);
    argv.insert(argv.end(), args);

    sendCommand(argv, callback);
}

void TairAsyncClient::scriptLoad(const std::string &script, const ResultStringCallback &callback) {
    sendCommand({"script", "load", script}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<BulkStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::scriptFlush(const ResultStringCallback &callback) {
    sendCommand({"script", "flush"}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::scriptKill(const ResultStringCallback &callback) {
    sendCommand({"script", "kill"}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::scriptExists(InitializerList<std::string> sha1s, const ResultVectorIntegerCallback &callback) {
    CommandArgv argv {"script", "exists"};
    argv.insert(argv.end(), sha1s);

    sendCommand(std::move(argv), [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackByBuilder<ArrayPacket, std::vector<int64_t>>(resp, TairResultHelper::vectorIntegerBuilder, callback);
    });
}

// -------------------------------- Connection Command --------------------------------
void TairAsyncClient::auth(const std::string &password, const ResultStringCallback &callback) {
    sendCommand({"auth", password}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::auth(const std::string &user, const std::string &password, const ResultStringCallback &callback) {
    sendCommand({"auth", user, password}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairAsyncClient::quit(const ResultStringCallback &callback) {
    sendCommand({"quit"}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

// -------------------------------- Server Command --------------------------------
void TairAsyncClient::flushall(const ResultStringCallback &callback) {
    sendCommand({"flushall"}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

// -------------------------------- Pubsub Command --------------------------------
void TairAsyncClient::publish(const std::string &channel, const std::string &message, const ResultIntegerCallback &callback) {
    sendCommand({"publish", channel, message}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

void TairAsyncClient::clusterPublish(const std::string &channel, const std::string &message,
                                     const std::string &name, int flag, const ResultIntegerCallback &callback) {
    sendCommand({"clusterpublish", channel, message, name, std::to_string(flag)}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<IntegerPacket, int64_t>(resp, callback);
    });
}

// -------------------------------- Cluster Command --------------------------------
void TairAsyncClient::clusterNodes(const ResultStringCallback &callback) {
    sendCommand({"cluster", "nodes"}, [callback](auto *, auto &, auto &resp) {
        TairResultHelper::doCallbackOneResult<BulkStringPacket, std::string>(resp, callback);
    });
}

} // namespace tair::client
