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
#include "client/TairClient.hpp"

#include "client/TairAsyncClient.hpp"
#include "client/TairClusterAsyncClient.hpp"

#include "common/Logger.hpp"

namespace tair::client {

TairClient::~TairClient() {
    destroy();
}

TairResult<std::string> TairClient::init(const TairURI &uri) {
    auto type = uri.getType();
    if (type == TairURI::STANDALONE) {
        LOG_INFO("Tair init in STANDALONE mode");
        itair_ = new TairAsyncClient(uri.getLoop());
    } else if (type == TairURI::CLUSTER) {
        LOG_INFO("Tair init in CLUSTER mode");
        itair_ = new TairClusterAsyncClient(uri.getLoop());
    } else if (type == TairURI::SENTINEL) {
        LOG_INFO("Tair init in SENTINEL mode, but we do not support, init failed.");
        return TairResult<std::string>::createErr("tair not support SENTINEL mode");
    }
    auto server_addrs = uri.getServerAddrs();
    if (type == TairURI::STANDALONE && server_addrs.size() != 1) {
        return TairResult<std::string>::createErr("STANDALONE mode not support multi addrs");
    }
    auto result = TairResult<std::string>::createErr("server_addrs is empty");
    for (const auto &server_addr : server_addrs) {
        if (server_addr.empty()) {
            return TairResult<std::string>::createErr("empty addr in server_addrs");
        }
        itair_->setServerAddr(server_addr);
        itair_->setConnectingTimeoutMs(uri.getConnectingTimeoutMs());
        itair_->setReconnectIntervalMs(uri.getReconnectIntervalMs());
        itair_->setAutoReconnect(uri.isAutoReconnect());
        if (!uri.getUser().empty()) {
            itair_->setUser(uri.getUser());
        }
        if (!uri.getPassword().empty()) {
            itair_->setPassword(uri.getPassword());
        }
        result = itair_->init();
        if (result.isSuccess()) {
            return result;
        }
    }
    return result;
}

void TairClient::destroy() {
    if (itair_) {
        itair_->destroy();
        delete itair_;
        itair_ = nullptr;
    }
}

TairClientWrapper TairClient::getFutureWrapper() {
    return TairClientWrapper(*this);
}

// -------------------------------- send Command --------------------------------
void TairClient::sendCommand(CommandArgv &&argv, const ResultPacketCallback &callback) {
    if (!itair_) {
        callback(nullptr, nullptr, nullptr);
    } else {
        itair_->sendCommand(argv, callback);
    }
}

void TairClient::sendCommand(const CommandArgv &argv, const ResultPacketCallback &callback) {
    if (!itair_) {
        callback(nullptr, nullptr, nullptr);
    } else {
        itair_->sendCommand(argv, callback);
    }
}

void TairClient::sendCommand(CommandArgv &&argv, const ResultPacketAndLatencyCallback &callback) {
    if (!itair_) {
        callback(nullptr, nullptr, nullptr, 0);
    } else {
        itair_->sendCommand(argv, callback);
    }
}

void TairClient::sendCommand(const CommandArgv &argv, const ResultPacketAndLatencyCallback &callback) {
    if (!itair_) {
        callback(nullptr, nullptr, nullptr, 0);
    } else {
        itair_->sendCommand(argv, callback);
    }
}

// -------------------------------- Generic Command --------------------------------
void TairClient::del(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->del(key, callback);
    }
}

void TairClient::del(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() == 0) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->del(keys, callback);
    }
}

void TairClient::unlink(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->unlink(key, callback);
    }
}

void TairClient::unlink(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() == 0) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->unlink(keys, callback);
    }
}

void TairClient::exists(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->exists(key, callback);
    }
}

void TairClient::exists(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() == 0) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->exists(keys, callback);
    }
}

void TairClient::expire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->expire(key, timeout, callback);
    }
}

void TairClient::expire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->expire(key, timeout, params, callback);
    }
}

void TairClient::expireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->expireat(key, timestamp, callback);
    }
}

void TairClient::expireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->expireat(key, timestamp, params, callback);
    }
}

void TairClient::persist(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->persist(key, callback);
    }
}

void TairClient::pexpire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->pexpire(key, timeout, callback);
    }
}

void TairClient::pexpire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->pexpire(key, timeout, params, callback);
    }
}

void TairClient::pexpireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->pexpireat(key, timestamp, callback);
    }
}

void TairClient::pexpireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->pexpireat(key, timestamp, params, callback);
    }
}

void TairClient::ttl(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->ttl(key, callback);
    }
}

void TairClient::pttl(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->pttl(key, callback);
    }
}

void TairClient::touch(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() == 0) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->touch(keys, callback);
    }
}

void TairClient::dump(const std::string &key, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->dump(key, callback);
    }
}

void TairClient::restore(const std::string &key, int64_t ttl, const std::string &value, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->restore(key, ttl, value, callback);
    }
}

void TairClient::restore(const std::string &key, int64_t ttl, const std::string &value, const RestoreParams &params, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->restore(key, ttl, value, params, callback);
    }
}

void TairClient::keys(const std::string &pattern, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->keys(pattern, callback);
    }
}

void TairClient::move(const std::string &key, int64_t db, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->move(key, db, callback);
    }
}

void TairClient::randomkey(const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->randomkey(callback);
    }
}

void TairClient::rename(const std::string &key, const std::string &newkey, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->rename(key, newkey, callback);
    }
}

void TairClient::renamenx(const std::string &key, const std::string &newkey, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->renamenx(key, newkey, callback);
    }
}

void TairClient::type(const std::string &key, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->type(key, callback);
    }
}

void TairClient::scan(const std::string &cursor, const ResultScanCallback &callback) {
    if (!itair_) {
        callback(TairResult<ScanResult>::createErr(E_NOT_INIT));
    } else {
        itair_->scan(cursor, callback);
    }
}

void TairClient::scan(const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    if (!itair_) {
        callback(TairResult<ScanResult>::createErr(E_NOT_INIT));
    } else {
        itair_->scan(cursor, params, callback);
    }
}

void TairClient::sort(const std::string &key, const ResultVectorStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_NOT_INIT));
    } else {
        itair_->sort(key, callback);
    }
}

void TairClient::sort(const std::string &key, const SortParams &params, const ResultVectorStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_NOT_INIT));
    } else {
        itair_->sort(key, params, callback);
    }
}

void TairClient::sortStore(const std::string &key, const std::string &storekey, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->sortStore(key, storekey, callback);
    }
}

void TairClient::sortStore(const std::string &key, const std::string &storekey, const SortParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->sortStore(key, storekey, params, callback);
    }
}

void TairClient::copy(const std::string &key, const std::string &destkey, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->copy(key, destkey, callback);
    }
}

void TairClient::copy(const std::string &key, const std::string &destkey, const CopyParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->copy(key, destkey, params, callback);
    }
}

// -------------------------------- String Command --------------------------------
void TairClient::append(const std::string &key, const std::string &value, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->append(key, value, callback);
    }
}

void TairClient::bitcount(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->bitcount(key, callback);
    }
}

void TairClient::bitcount(const std::string &key, const BitPositonParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->bitcount(key, params, callback);
    }
}

void TairClient::bitfield(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::shared_ptr<int64_t>>>::createErr(E_NOT_INIT));
    } else {
        itair_->bitfield(key, args, callback);
    }
}

void TairClient::bitfieldRo(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::shared_ptr<int64_t>>>::createErr(E_NOT_INIT));
    } else {
        itair_->bitfieldRo(key, args, callback);
    }
}

void TairClient::bitop(const BitOperation &op, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<int64_t>::createErr("the number of keys is less than 2"));
    } else {
        itair_->bitop(op, keys, callback);
    }
}

void TairClient::bitpos(const std::string &key, int64_t bit, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->bitpos(key, bit, callback);
    }
}

void TairClient::bitpos(const std::string &key, int64_t bit, const BitPositonParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->bitpos(key, bit, params, callback);
    }
}

void TairClient::setbit(const std::string &key, int64_t offset, int64_t value, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->setbit(key, offset, value, callback);
    }
}

void TairClient::getbit(const std::string &key, int64_t offset, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->getbit(key, offset, callback);
    }
}

void TairClient::decr(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->decr(key, callback);
    }
}

void TairClient::decrby(const std::string &key, int64_t decrement, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->decrby(key, decrement, callback);
    }
}

void TairClient::getrange(const std::string &key, int64_t start, int64_t end, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->getrange(key, start, end, callback);
    }
}

void TairClient::getset(const std::string &key, const std::string &value, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->getset(key, value, callback);
    }
}

void TairClient::set(const std::string &key, const std::string &value, const SetParams &params, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->set(key, value, params, callback);
    }
}

void TairClient::set(const std::string &key, const std::string &value, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->set(key, value, callback);
    }
}

void TairClient::get(const std::string &key, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->get(key, callback);
    }
}

void TairClient::incr(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->incr(key, callback);
    }
}

void TairClient::incrby(const std::string &key, int64_t increment, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->incrby(key, increment, callback);
    }
}

void TairClient::incrbyfloat(const std::string &key, double increment, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->incrbyfloat(key, increment, callback);
    }
}

void TairClient::mget(InitializerList<std::string> keys, const ResultVectorStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_NOT_INIT));
    } else if (keys.size() == 0) {
        callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->mget(keys, callback);
    }
}

void TairClient::mset(InitializerList<std::string> kvs, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else if (kvs.size() < 2) {
        callback(TairResult<std::string>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->mset(kvs, callback);
    }
}

void TairClient::msetnx(InitializerList<std::string> kvs, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (kvs.size() < 2) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->msetnx(kvs, callback);
    }
}

void TairClient::psetex(const std::string &key, int64_t milliseconds, const std::string &value, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->psetex(key, milliseconds, value, callback);
    }
}

void TairClient::setex(const std::string &key, int64_t seconds, const std::string &value, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->setex(key, seconds, value, callback);
    }
}

void TairClient::setnx(const std::string &key, const std::string &value, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->setnx(key, value, callback);
    }
}

void TairClient::setrange(const std::string &key, int64_t offset, const std::string &value, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->setrange(key, offset, value, callback);
    }
}

void TairClient::strlen(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->strlen(key, callback);
    }
}

void TairClient::getdel(const std::string &key, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->getdel(key, callback);
    }
}

void TairClient::getex(const std::string &key, const GetExParams &params, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->getex(key, params, callback);
    }
}

// -------------------------------- List Command --------------------------------
void TairClient::blpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->blpop(keys, timeout, callback);
    }
}

void TairClient::brpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->brpop(keys, timeout, callback);
    }
}

void TairClient::brpoplpush(const std::string &src, const std::string &dest, int64_t timeout, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->brpoplpush(src, dest, timeout, callback);
    }
}

void TairClient::lindex(const std::string &key, int64_t index, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->lindex(key, index, callback);
    }
}

void TairClient::linsert(const std::string &key, const ListDirection &direction, const std::string &pivot, const std::string &element, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->linsert(key, direction, pivot, element, callback);
    }
}

void TairClient::llen(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->llen(key, callback);
    }
}

void TairClient::lpop(const std::string &key, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->lpop(key, callback);
    }
}

void TairClient::lpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->lpop(key, count, callback);
    }
}

void TairClient::lpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (elements.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->lpush(key, elements, callback);
    }
}

void TairClient::lpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (elements.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->lpushx(key, elements, callback);
    }
}

void TairClient::lrange(const std::string &key, int64_t start, int64_t stop, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->lrange(key, start, stop, callback);
    }
}

void TairClient::lrem(const std::string &key, int64_t count, const std::string &element, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->lrem(key, count, element, callback);
    }
}

void TairClient::lset(const std::string &key, int64_t index, const std::string &element, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->lset(key, index, element, callback);
    }
}

void TairClient::ltrim(const std::string &key, int64_t start, int64_t stop, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->ltrim(key, start, stop, callback);
    }
}

void TairClient::rpop(const std::string &key, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->rpop(key, callback);
    }
}

void TairClient::rpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->rpop(key, count, callback);
    }
}

void TairClient::rpoplpush(const std::string &src, const std::string &dest, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->rpoplpush(src, dest, callback);
    }
}

void TairClient::rpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (elements.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->rpush(key, elements, callback);
    }
}

void TairClient::rpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (elements.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->rpushx(key, elements, callback);
    }
}

void TairClient::lmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->lmove(src, dest, ld, rd, callback);
    }
}

void TairClient::blmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, int64_t timeout, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->blmove(src, dest, ld, rd, timeout, callback);
    }
}

// -------------------------------- Set Command --------------------------------
void TairClient::sadd(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (members.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->sadd(key, members, callback);
    }
}

void TairClient::scard(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->scard(key, callback);
    }
}

void TairClient::sismember(const std::string &key, const std::string &member, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->sismember(key, member, callback);
    }
}

void TairClient::smembers(const std::string &key, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->smembers(key, callback);
    }
}

void TairClient::smove(const std::string &src, const std::string &dest, const std::string &member, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->smove(src, dest, member, callback);
    }
}

void TairClient::srem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (members.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->srem(key, members, callback);
    }
}

void TairClient::spop(const std::string &key, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->spop(key, callback);
    }
}

void TairClient::spop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->spop(key, count, callback);
    }
}

void TairClient::srandmember(const std::string &key, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->srandmember(key, callback);
    }
}

void TairClient::srandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->srandmember(key, count, callback);
    }
}

void TairClient::sdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->sdiff(keys, callback);
    }
}

void TairClient::sinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->sinter(keys, callback);
    }
}

void TairClient::sunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->sunion(keys, callback);
    }
}

void TairClient::sdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->sdiffstore(dest, keys, callback);
    }
}

void TairClient::sinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->sinterstore(dest, keys, callback);
    }
}

void TairClient::sunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->sunionstore(dest, keys, callback);
    }
}

void TairClient::sscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) {
    if (!itair_) {
        callback(TairResult<ScanResult>::createErr(E_NOT_INIT));
    } else {
        itair_->sscan(key, cursor, callback);
    }
}

void TairClient::sscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    if (!itair_) {
        callback(TairResult<ScanResult>::createErr(E_NOT_INIT));
    } else {
        itair_->sscan(key, cursor, params, callback);
    }
}

void TairClient::smismember(const std::string &key, InitializerList<std::string> members, const ResultVectorIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<int64_t>>::createErr(E_NOT_INIT));
    } else if (members.size() < 1) {
        callback(TairResult<std::vector<int64_t>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->smismember(key, members, callback);
    }
}

// -------------------------------- Hash Command --------------------------------
void TairClient::hdel(const std::string &key, InitializerList<std::string> fields, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (fields.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->hdel(key, fields, callback);
    }
}

void TairClient::hexists(const std::string &key, const std::string &field, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->hexists(key, field, callback);
    }
}

void TairClient::hget(const std::string &key, const std::string &field, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->hget(key, field, callback);
    }
}

void TairClient::hgetall(const std::string &key, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->hgetall(key, callback);
    }
}

void TairClient::hincrby(const std::string &key, const std::string &field, int64_t increment, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->hincrby(key, field, increment, callback);
    }
}

void TairClient::hincrbyfloat(const std::string &key, const std::string &field, double increment, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->hincrbyfloat(key, field, increment, callback);
    }
}

void TairClient::hkeys(const std::string &key, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->hkeys(key, callback);
    }
}

void TairClient::hvals(const std::string &key, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->hvals(key, callback);
    }
}

void TairClient::hlen(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->hlen(key, callback);
    }
}

void TairClient::hmget(const std::string &key, InitializerList<std::string> fields, const ResultVectorStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_NOT_INIT));
    } else if (fields.size() < 1) {
        callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->hmget(key, fields, callback);
    }
}

void TairClient::hset(const std::string &key, const std::string &filed, const std::string &value, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->hset(key, filed, value, callback);
    }
}

void TairClient::hset(const std::string &key, InitializerList<std::string> kvs, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (kvs.size() < 2) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->hset(key, kvs, callback);
    }
}

void TairClient::hsetnx(const std::string &key, const std::string &field, const std::string &value, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->hsetnx(key, field, value, callback);
    }
}

void TairClient::hstrlen(const std::string &key, const std::string &field, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->hstrlen(key, field, callback);
    }
}

void TairClient::hscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) {
    if (!itair_) {
        callback(TairResult<ScanResult>::createErr(E_NOT_INIT));
    } else {
        itair_->hscan(key, cursor, callback);
    }
}

void TairClient::hscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    if (!itair_) {
        callback(TairResult<ScanResult>::createErr(E_NOT_INIT));
    } else {
        itair_->hscan(key, cursor, params, callback);
    }
}

void TairClient::hrandfield(const std::string &key, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->hrandfield(key, callback);
    }
}

void TairClient::hrandfield(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->hrandfield(key, count, callback);
    }
}

// -------------------------------- Zset Command --------------------------------
void TairClient::zadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (elements.size() < 2) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zadd(key, elements, callback);
    }
}

void TairClient::zadd(const std::string &key, const ZAddParams &params, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (elements.size() < 2) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zadd(key, params, elements, callback);
    }
}

void TairClient::zincrby(const std::string &key, int64_t increment, const std::string &member, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->zincrby(key, increment, member, callback);
    }
}

void TairClient::zscore(const std::string &key, const std::string &member, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->zscore(key, member, callback);
    }
}

void TairClient::zrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<int64_t>>::createErr(E_NOT_INIT));
    } else {
        itair_->zrank(key, member, callback);
    }
}

void TairClient::zrevrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<int64_t>>::createErr(E_NOT_INIT));
    } else {
        itair_->zrevrank(key, member, callback);
    }
}

void TairClient::zcard(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->zcard(key, callback);
    }
}

void TairClient::zcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->zcount(key, min, max, callback);
    }
}

void TairClient::zlexcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->zlexcount(key, min, max, callback);
    }
}

void TairClient::zpopmax(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->zpopmax(key, count, callback);
    }
}

void TairClient::zpopmin(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->zpopmin(key, count, callback);
    }
}

void TairClient::bzpopmax(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->bzpopmax(keys, timeout, callback);
    }
}

void TairClient::bzpopmin(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->bzpopmin(keys, timeout, callback);
    }
}

void TairClient::zrange(const std::string &key, const std::string &start, const std::string &stop, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->zrange(key, start, stop, callback);
    }
}

void TairClient::zrange(const std::string &key, const std::string &start, const std::string &stop, const ZRangeParams &params, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->zrange(key, start, stop, params, callback);
    }
}

void TairClient::zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->zrangestore(dest, src, min, max, callback);
    }
}

void TairClient::zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ZRangeParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->zrangestore(dest, src, min, max, params, callback);
    }
}

void TairClient::zrem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (members.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zrem(key, members, callback);
    }
}

void TairClient::zremrange(const ZRemRangeOption &option, const std::string &key, const std::string &begin, const std::string &end, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->zremrange(option, key, begin, end, callback);
    }
}

void TairClient::zinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zinter(keys, callback);
    }
}

void TairClient::zinter(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zinter(keys, params, callback);
    }
}

void TairClient::zinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zinterstore(dest, keys, callback);
    }
}

void TairClient::zinterstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zinterstore(dest, keys, params, callback);
    }
}

void TairClient::zunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zunion(keys, callback);
    }
}

void TairClient::zunion(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zunion(keys, params, callback);
    }
}

void TairClient::zunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zunionstore(dest, keys, callback);
    }
}

void TairClient::zunionstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zunionstore(dest, keys, params, callback);
    }
}

void TairClient::zdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zdiff(keys, callback);
    }
}

void TairClient::zdiffWithScores(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<std::vector<std::string>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zdiffWithScores(keys, callback);
    }
}

void TairClient::zdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zdiffstore(dest, keys, callback);
    }
}

void TairClient::zdiffstoreWithScores(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 2) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zdiffstoreWithScores(dest, keys, callback);
    }
}

void TairClient::zmscore(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_NOT_INIT));
    } else if (members.size() < 1) {
        callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->zmscore(key, members, callback);
    }
}

void TairClient::zrandmember(const std::string &key, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->zrandmember(key, callback);
    }
}

void TairClient::zrandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->zrandmember(key, count, callback);
    }
}

void TairClient::zscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) {
    if (!itair_) {
        callback(TairResult<ScanResult>::createErr(E_NOT_INIT));
    } else {
        itair_->zscan(key, cursor, callback);
    }
}

void TairClient::zscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    if (!itair_) {
        callback(TairResult<ScanResult>::createErr(E_NOT_INIT));
    } else {
        itair_->zscan(key, cursor, params, callback);
    }
}

// -------------------------------- HyperLogLog Command --------------------------------
void TairClient::pfadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (elements.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->pfadd(key, elements, callback);
    }
}

void TairClient::pfcount(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->pfcount(keys, callback);
    }
}

void TairClient::pfmerge(const std::string &dest, InitializerList<std::string> keys, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1) {
        callback(TairResult<std::string>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->pfmerge(dest, keys, callback);
    }
}

// -------------------------------- Geo Command --------------------------------
void TairClient::geoadd(const std::string &key, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->geoadd(key, members, callback);
    }
}

void TairClient::geoadd(const std::string &key, const GeoAddParams &params, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->geoadd(key, params, members, callback);
    }
}

void TairClient::geodist(const std::string &key, const std::string &member1, const std::string &member2, const GeoUnit &unit, const ResultStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->geodist(key, member1, member2, unit, callback);
    }
}

void TairClient::geohash(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_NOT_INIT));
    } else if (members.size() < 1) {
        callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->geohash(key, members, callback);
    }
}

void TairClient::geopos(const std::string &key, InitializerList<std::string> members, const ResultGeoposCallback &callback) {
    if (!itair_) {
        callback(TairResult<GeoPosResult>::createErr(E_NOT_INIT));
    } else if (members.size() < 1) {
        callback(TairResult<GeoPosResult>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->geopos(key, members, callback);
    }
}

void TairClient::georadius(const std::string &key, double longitude, double latitude, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->georadius(key, longitude, latitude, radius, unit, callback);
    }
}

void TairClient::georadiusbymember(const std::string &key, const std::string &member, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_INIT));
    } else {
        itair_->georadiusbymember(key, member, radius, unit, callback);
    }
}

// -------------------------------- Stream Command --------------------------------
void TairClient::xadd(const std::string &key, const std::string &id, InitializerList<std::string> elements, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else if (elements.size() < 1) {
        callback(TairResult<std::string>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->xadd(key, id, elements, callback);
    }
}

void TairClient::xlen(const std::string &key, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->xlen(key, callback);
    }
}

void TairClient::xdel(const std::string &key, InitializerList<std::string> ids, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (ids.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->xdel(key, ids, callback);
    }
}

void TairClient::xack(const std::string &key, const std::string &group, InitializerList<std::string> ids, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else if (ids.size() < 1) {
        callback(TairResult<int64_t>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->xack(key, group, ids, callback);
    }
}

void TairClient::xgroupCreate(const std::string &key, const std::string &group, const std::string &id, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->xgroupCreate(key, group, id, callback);
    }
}

void TairClient::xgroupCreateConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->xgroupCreateConsumer(key, group, consumer, callback);
    }
}

void TairClient::xgroupDelConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->xgroupDelConsumer(key, group, consumer, callback);
    }
}

void TairClient::xgroupDestroy(const std::string &key, const std::string &group, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->xgroupDestroy(key, group, callback);
    }
}

void TairClient::xgroupSetID(const std::string &key, const std::string &group, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->xgroupSetID(key, group, callback);
    }
}

void TairClient::xpending(const std::string &key, const std::string &group, const ResultXPendingCallback &callback) {
    if (!itair_) {
        callback(TairResult<XPendingResult>::createErr(E_NOT_INIT));
    } else {
        itair_->xpending(key, group, callback);
    }
}

void TairClient::xrange(const std::string &key, const std::string &start, const std::string &end, const ResultXRangeCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<XRangeResult>>::createErr(E_NOT_INIT));
    } else {
        itair_->xrange(key, start, end, callback);
    }
}

void TairClient::xrevrange(const std::string &key, const std::string &end, const std::string &start, const ResultXRangeCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<XRangeResult>>::createErr(E_NOT_INIT));
    } else {
        itair_->xrevrange(key, end, start, callback);
    }
}

void TairClient::xread(int64_t count, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<XReadResult>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1 || ids.size() < 1) {
        callback(TairResult<std::vector<XReadResult>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->xread(count, keys, ids, callback);
    }
}

void TairClient::xreadgroup(const std::string &group, const std::string &consumer, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<XReadResult>>::createErr(E_NOT_INIT));
    } else if (keys.size() < 1 || ids.size() < 1) {
        callback(TairResult<std::vector<XReadResult>>::createErr(E_PARAMS_EMPTY));
    } else {
        itair_->xreadgroup(group, consumer, keys, ids, callback);
    }
}

void TairClient::xtrim(const std::string &key, const std::string &strategy, int64_t threshold, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->xtrim(key, strategy, threshold, callback);
    }
}

void TairClient::xsetid(const std::string &key, const std::string &last_id, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->xsetid(key, last_id, callback);
    }
}

// -------------------------------- Script Command --------------------------------
void TairClient::eval(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    if (!itair_) {
        callback(nullptr, nullptr, nullptr);
    } else {
        itair_->eval(script, keys, args, callback);
    }
}

void TairClient::evalRo(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    if (!itair_) {
        callback(nullptr, nullptr, nullptr);
    } else {
        itair_->evalRo(script, keys, args, callback);
    }
}

void TairClient::evalsha(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    if (!itair_) {
        callback(nullptr, nullptr, nullptr);
    } else {
        itair_->evalsha(sha1, keys, args, callback);
    }
}

void TairClient::evalshaRo(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    if (!itair_) {
        callback(nullptr, nullptr, nullptr);
    } else {
        itair_->evalshaRo(sha1, keys, args, callback);
    }
}

void TairClient::scriptLoad(const std::string &script, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->scriptLoad(script, callback);
    }
}

void TairClient::scriptFlush(const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->scriptFlush(callback);
    }
}

void TairClient::scriptKill(const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->scriptKill(callback);
    }
}

void TairClient::scriptExists(InitializerList<std::string> sha1s, const ResultVectorIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::vector<int64_t>>::createErr(E_NOT_INIT));
    } else {
        itair_->scriptExists(sha1s, callback);
    }
}

// -------------------------------- Connection Command --------------------------------
void TairClient::auth(const std::string &password, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->auth(password, callback);
    }
}

void TairClient::auth(const std::string &user, const std::string &password, const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->auth(user, password, callback);
    }
}

void TairClient::quit(const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->quit(callback);
    }
}

// -------------------------------- Server Command --------------------------------
void TairClient::flushall(const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->flushall(callback);
    }
}

// -------------------------------- Pubsub Command --------------------------------
void TairClient::publish(const std::string &channel, const std::string &message, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->publish(channel, message, callback);
    }
}

void TairClient::clusterPublish(const std::string &channel, const std::string &message, const std::string &name, int flag, const ResultIntegerCallback &callback) {
    if (!itair_) {
        callback(TairResult<int64_t>::createErr(E_NOT_INIT));
    } else {
        itair_->clusterPublish(channel, message, name, flag, callback);
    }
}

// -------------------------------- Cluster Command --------------------------------
void TairClient::clusterNodes(const ResultStringCallback &callback) {
    if (!itair_) {
        callback(TairResult<std::string>::createErr(E_NOT_INIT));
    } else {
        itair_->clusterNodes(callback);
    }
}

} // namespace tair::client