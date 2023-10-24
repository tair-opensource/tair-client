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
#include "client/TairClusterAsyncClient.hpp"

#include "common/ClockTime.hpp"
#include "common/StringUtil.hpp"
#include "network/EventLoopThread.hpp"
#include "client/TairResultHelper.hpp"

#include "absl/strings/numbers.h"

namespace tair::client {

using common::ClockTime;
using common::StringUtil;
using protocol::BulkStringPacket;
using protocol::IntegerPacket;
using protocol::SimpleStringPacket;
using client::TairResultHelper;

using absl::SimpleAtoi;

TairClusterAsyncClient::TairClusterAsyncClient() {
    initEventLoop();
}

TairClusterAsyncClient::TairClusterAsyncClient(EventLoop *loop) {
    loop_ = loop;
    if (!loop_) {
        initEventLoop();
    }
}

TairClusterAsyncClient::~TairClusterAsyncClient() {
    destroy();
    if (loop_thread_) {
        loop_thread_->stop();
        loop_thread_->join();
        loop_thread_.reset();
    }
}

void TairClusterAsyncClient::initEventLoop() {
    loop_thread_ = std::make_unique<EventLoopThread>("client-io");
    loop_thread_->start();
    loop_ = loop_thread_->loop();
}

TairResult<std::string> TairClusterAsyncClient::init() {
    TairResult<std::string> result;
    auto client = createClient(server_addr_);
    if (!client) {
        result.setErr("connect to server failed");
        return result;
    }
    std::string nodes_info;
    if (!getClusterNodesInfo(client, nodes_info)) {
        client_map_.clear();
        result.setErr("get cluster nodes info failed");
        return result;
    }
    if (!parseNodesInfoAndInitClient(nodes_info)) {
        client_map_.clear();
        result.setErr("parse cluster nodes info failed");
        return result;
    }
    if (!checkSlotToClients()) {
        result.setErr("some slots are not initialized");
        return result;
    }

    result.setValue("ok");
    return result;
}

void TairClusterAsyncClient::destroy() {
    for (auto [_, client] : client_map_) {
        client->disconnect();
    }
    client_map_.clear();
    slot_to_clients_.fill(nullptr);
}

void TairClusterAsyncClient::setServerAddr(const std::string &addr) {
    server_addr_ = addr;
}

const std::string &TairClusterAsyncClient::getServerAddr() const {
    return server_addr_;
}

void TairClusterAsyncClient::setUser(const std::string &user) {
    user_ = user;
}

void TairClusterAsyncClient::setPassword(const std::string &password) {
    password_ = password;
}

void TairClusterAsyncClient::setConnectingTimeoutMs(int timeout_ms) {
    connecting_timeout_ms_ = timeout_ms;
}

void TairClusterAsyncClient::setReconnectIntervalMs(int timeout_ms) {
    reconnect_interval_ms_ = timeout_ms;
}

void TairClusterAsyncClient::setAutoReconnect(bool reconnect) {
    auto_reconnect_ = reconnect;
}

void TairClusterAsyncClient::setKeepAliveSeconds(int seconds) {
    keepalive_seconds_ = seconds;
}

bool TairClusterAsyncClient::checkResultHasClusterError(const PacketPtr &resp) {
    // Check for ask or moved errors here.
    // But it is not implemented now.
    // The upper layer code is required to reinitialize the client.
    return false;
}

int TairClusterAsyncClient::calcCommandSlot(const CommandArgv &argv) {
    size_t cmd_index = 0, key_index = 1;
    if (StringUtil::equalsNoCase(argv[0], "ars")) {
        cmd_index = 2;
        key_index = 3;
    }
    if (StringUtil::equalsNoCase(argv[cmd_index], "bitop") || StringUtil::equalsNoCase(argv[cmd_index], "xgroup")) {
        key_index = cmd_index + 2;
    } else if (StringUtil::equalsNoCase(argv[cmd_index], "xreadgroup") || StringUtil::equalsNoCase(argv[cmd_index], "xread")) {
        bool found = false;
        for (size_t i = cmd_index + 1; i < argv.size(); i++) {
            if (StringUtil::equalsNoCase(argv[i], "streams") && i + 1 < argv.size()) {
                key_index = i + 1;
                found = true;
                break;
            }
        }
        if (!found) {
            return -1;
        }
    }
    if (key_index >= argv.size()) {
        return -1;
    }
    return KeyHash::keyHashSlot(argv[key_index]);
}

TairAsyncClientPtr TairClusterAsyncClient::getClientByKey(const std::string &key) {
    uint16_t slot = KeyHash::keyHashSlot((key));
    return slot_to_clients_[slot];
}

TairAsyncClientPtr TairClusterAsyncClient::getClientRandom() {
    uint16_t slot = ::time(nullptr) % KeyHash::SLOTS_NUM;
    return slot_to_clients_[slot];
}

bool TairClusterAsyncClient::checkSlotToClients() {
    for (uint16_t i = 0; i < KeyHash::SLOTS_NUM; ++i) {
        if (!slot_to_clients_[i]) {
            LOG_ERROR("FATAL: cannot found slot[{}] in cluster nodes", i);
            return false;
        }
    }
    return true;
}

bool TairClusterAsyncClient::parseNodesInfoAndInitClient(std::string &nodes_info) {
    auto node_lines = StringUtil::split(nodes_info, '\n');
    for (const auto &line : node_lines) {
        auto items = StringUtil::split(line, ' ');
        if (items.size() < 9) {
            continue;
        }
        const auto &flags = items[2];
        // Only parser master line
        if (flags.find("master") == std::string::npos) {
            continue;
        }
        std::string server_addr;
        auto &addr = items[1];
        auto result = StringUtil::split(addr, '@');
        if (result.size() == 2 || result.size() == 1) { // result[1] is cport, ignore it
            server_addr = result[0];
        } else {
            LOG_ERROR("FATAL: unknown server addr format, cluster info line : {}", line);
            return false;
        }
        auto client = createClient(server_addr);
        if (!client) {
            LOG_ERROR("FATAL: unknown server addr format, cluster info line : {}", line);
            return false;
        }
        for (size_t slot_index = 8; slot_index < items.size(); ++slot_index) {
            auto &slot_info = items[slot_index];
            if (slot_info.empty() || slot_info[0] == '[') {
                break;
            }
            auto range_pair = StringUtil::split(slot_info, '-');
            if (range_pair.size() == 2) {
                int range_start, range_end;
                if (!SimpleAtoi(range_pair[0], &range_start) || !SimpleAtoi(range_pair[1], &range_end)
                    || range_start < 0 || range_start >= (int)KeyHash::SLOTS_NUM || range_end < 0
                    || range_end >= (int)KeyHash::SLOTS_NUM || range_start > range_end) {
                    LOG_ERROR("FATAL: unknown slot range format, cluster info line : {}", line);
                    return false;
                } else {
                    for (int i = range_start; i <= range_end; ++i) {
                        slot_to_clients_[i] = client;
                    }
                }
            } else if (range_pair.size() == 1) {
                int slot;
                if (!SimpleAtoi(range_pair[0], &slot) || slot < 0 || slot >= (int)KeyHash::SLOTS_NUM) {
                    LOG_ERROR("FATAL: unknown slot range format, cluster info line : {}", line);
                    return false;
                } else {
                    slot_to_clients_[slot] = client;
                }
            } else {
                LOG_ERROR("FATAL: unknown slot range format, cluster info line : {}", line);
                return false;
            }
        }
    }
    return true;
}

bool TairClusterAsyncClient::getClusterNodesInfo(const TairAsyncClientPtr &client, std::string &nodes_info) {
    auto promise = std::make_shared<std::promise<TairResult<std::string>>>();
    auto future = promise->get_future();
    client->clusterNodes([promise](auto &result) {
        promise->set_value(result);
    });
    std::string err;
    if (!TairResultHelper::waitFuture("cluster-nodes", future, nodes_info, err, connecting_timeout_ms_)) {
        LOG_ERROR("FATAL: call cluster nodes failed: {}", err);
        return false;
    }
    return true;
}

TairAsyncClientPtr TairClusterAsyncClient::createClient(const std::string &addr) {
    auto iter = client_map_.find(addr);
    if (iter != client_map_.end()) {
        return iter->second;
    }
    auto client = std::make_shared<TairAsyncClient>(loop_);
    client->setServerAddr(addr);
    client->setConnectingTimeoutMs(connecting_timeout_ms_);
    client->setReconnectIntervalMs(reconnect_interval_ms_);
    client->setKeepAliveSeconds(keepalive_seconds_);
    client->setAutoReconnect(auto_reconnect_);
    client->setUser(user_);
    client->setPassword(password_);
    TairResult<std::string> result = client->connect().get();
    if (!result.isSuccess()) {
        LOG_ERROR("FATAL: connect to server failed: {}", result.getErr());
        return nullptr;
    }
    client_map_.emplace(addr, client);
    return client;
}

bool TairClusterAsyncClient::checkKeyInSameSlot(std::initializer_list<std::string> list) {
    if (list.size() == 0) {
        return true;
    }
    uint16_t first_slot = KeyHash::keyHashSlot(*list.begin());
    for (auto &key : list) {
        uint16_t slot = KeyHash::keyHashSlot(key);
        if (slot != first_slot) {
            return false;
        }
    }
    return true;
}

bool TairClusterAsyncClient::checkKeyInSameSlot(const std::string &dest, std::initializer_list<std::string> list) {
    uint16_t first_slot = KeyHash::keyHashSlot(dest);
    for (auto &key : list) {
        uint16_t slot = KeyHash::keyHashSlot(key);
        if (slot != first_slot) {
            return false;
        }
    }
    return true;
}

// -------------------------------- send Command --------------------------------
void TairClusterAsyncClient::sendCommand(CommandArgv &&argv, const ResultPacketCallback &callback) {
    int slot = calcCommandSlot(argv);
    if (slot < 0 || !slot_to_clients_[slot]) {
        callback(nullptr, nullptr, nullptr);
    }
    auto tair_client = slot_to_clients_[slot];
    tair_client->sendCommand(std::move(argv), [this, callback](auto *client, auto &req, auto &resp) {
        if (!checkResultHasClusterError(resp)) {
            callback(client, req, resp);
        }
    });
}

void TairClusterAsyncClient::sendCommand(const CommandArgv &argv, const ResultPacketCallback &callback) {
    int slot = calcCommandSlot(argv);
    if (slot < 0 || !slot_to_clients_[slot]) {
        callback(nullptr, nullptr, nullptr);
    }
    auto tair_client = slot_to_clients_[slot];
    tair_client->sendCommand(argv, [this, callback](auto *client, auto &req, auto &resp) {
        if (!checkResultHasClusterError(resp)) {
            callback(client, req, resp);
        }
    });
}

void TairClusterAsyncClient::sendCommand(CommandArgv &&argv, const ResultPacketAndLatencyCallback &callback) {
    int slot = calcCommandSlot(argv);
    if (slot < 0 || !slot_to_clients_[slot]) {
        callback(nullptr, nullptr, nullptr, 0);
    }
    auto tair_client = slot_to_clients_[slot];
    tair_client->sendCommand(std::move(argv), [this, callback](auto *client, auto &req, auto &resp, int64_t latency_us) {
        if (!checkResultHasClusterError(resp)) {
            callback(client, req, resp, latency_us);
        }
    });
}

void TairClusterAsyncClient::sendCommand(const CommandArgv &argv, const ResultPacketAndLatencyCallback &callback) {
    int slot = calcCommandSlot(argv);
    if (slot < 0 || !slot_to_clients_[slot]) {
        callback(nullptr, nullptr, nullptr, 0);
    }
    auto tair_client = slot_to_clients_[slot];
    tair_client->sendCommand(argv, [this, callback](auto *client, auto &req, auto &resp, int64_t latency_us) {
        if (!checkResultHasClusterError(resp)) {
            callback(client, req, resp, latency_us);
        }
    });
}

// -------------------------------- Generic Command --------------------------------
void TairClusterAsyncClient::del(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->del(key, callback);
}

void TairClusterAsyncClient::del(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    callback(TairResult<int64_t>::createErr(E_CLUSTER_NOT_SUPPORT_COMMAND));
}

void TairClusterAsyncClient::unlink(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->unlink(key, callback);
}

void TairClusterAsyncClient::unlink(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(*keys.begin());
    client->unlink(keys, callback);
}

void TairClusterAsyncClient::exists(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->exists(key, callback);
}

void TairClusterAsyncClient::exists(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(*keys.begin());
    client->exists(keys, callback);
}

void TairClusterAsyncClient::expire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->expire(key, timeout, callback);
}

void TairClusterAsyncClient::expire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->expire(key, timeout, params, callback);
}

void TairClusterAsyncClient::expireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->expireat(key, timestamp, callback);
}

void TairClusterAsyncClient::expireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->expireat(key, timestamp, params, callback);
}

void TairClusterAsyncClient::persist(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->persist(key, callback);
}

void TairClusterAsyncClient::pexpire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->pexpire(key, timeout, callback);
}

void TairClusterAsyncClient::pexpire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->pexpire(key, timeout, params, callback);
}

void TairClusterAsyncClient::pexpireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->pexpireat(key, timestamp, callback);
}

void TairClusterAsyncClient::pexpireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->pexpireat(key, timestamp, params, callback);
}

void TairClusterAsyncClient::ttl(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->ttl(key, callback);
}

void TairClusterAsyncClient::pttl(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->pttl(key, callback);
}

void TairClusterAsyncClient::touch(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(*keys.begin());
    client->touch(keys, callback);
}

void TairClusterAsyncClient::dump(const std::string &key, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->dump(key, callback);
}

void TairClusterAsyncClient::restore(const std::string &key, int64_t ttl, const std::string &value, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->restore(key, ttl, value, callback);
}

void TairClusterAsyncClient::restore(const std::string &key, int64_t ttl, const std::string &value, const RestoreParams &params, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->restore(key, ttl, value, params, callback);
}

void TairClusterAsyncClient::keys(const std::string &pattern, const ResultVectorStringCallback &callback) {
    bool ok = true;
    std::vector<std::string> all_keys;
    CountDownLatch latch(client_map_.size());
    for (const auto &n : client_map_) {
        auto server_addr = n.first;
        auto clientPtr = n.second;
        clientPtr->keys(pattern, [&](const TairResult<std::vector<std::string>> &result) {
            latch.countDown();
            if (!result.isSuccess()) {
                ok = false;
                LOG_ERROR("cluster call keys failed, node: {}, err: {}", server_addr, result.getErr());
            } else {
                auto part_keys = result.getValue();
                all_keys.insert(all_keys.end(), part_keys.begin(), part_keys.end());
            }
        });
    }
    latch.wait();
    if (ok) {
        callback(TairResult<std::vector<std::string>>::create(std::move(all_keys)));
    } else {
        callback(TairResult<std::vector<std::string>>::createErr("keys failed"));
    }
}

void TairClusterAsyncClient::move(const std::string &key, int64_t db, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->move(key, db, callback);
}

void TairClusterAsyncClient::randomkey(const ResultStringPtrCallback &callback) {
    auto client = getClientRandom();
    client->randomkey(callback);
}

void TairClusterAsyncClient::rename(const std::string &key, const std::string &newkey, const ResultStringCallback &callback) {
    if (!checkKeyInSameSlot({key, newkey})) {
        callback(TairResult<std::string>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(key);
    client->rename(key, newkey, callback);
}

void TairClusterAsyncClient::renamenx(const std::string &key, const std::string &newkey, const ResultIntegerCallback &callback) {
    if (!checkKeyInSameSlot({key, newkey})) {
        callback(TairResult<int64_t>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(key);
    client->renamenx(key, newkey, callback);
}

void TairClusterAsyncClient::type(const std::string &key, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->type(key, callback);
}

void TairClusterAsyncClient::scan(const std::string &cursor, const ResultScanCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::scan(const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::sort(const std::string &key, const ResultVectorStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->sort(key, callback);
}

void TairClusterAsyncClient::sort(const std::string &key, const SortParams &params, const ResultVectorStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->sort(key, params, callback);
}

void TairClusterAsyncClient::sortStore(const std::string &key, const std::string &storekey, const ResultIntegerCallback &callback) {
    if (!checkKeyInSameSlot({key, storekey})) {
        callback(TairResult<int64_t>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(key);
    client->sortStore(key, storekey, callback);
}

void TairClusterAsyncClient::sortStore(const std::string &key, const std::string &storekey, const SortParams &params, const ResultIntegerCallback &callback) {
    if (!checkKeyInSameSlot({key, storekey})) {
        callback(TairResult<int64_t>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(key);
    client->sortStore(key, storekey, params, callback);
}

void TairClusterAsyncClient::copy(const std::string &key, const std::string &destkey, const ResultIntegerCallback &callback) {
    if (!checkKeyInSameSlot({key, destkey})) {
        callback(TairResult<int64_t>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(key);
    client->copy(key, destkey, callback);
}

void TairClusterAsyncClient::copy(const std::string &key, const std::string &destkey, const CopyParams &params, const ResultIntegerCallback &callback) {
    if (!checkKeyInSameSlot({key, destkey})) {
        callback(TairResult<int64_t>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(key);
    client->copy(key, destkey, params, callback);
}

// -------------------------------- String Command --------------------------------
void TairClusterAsyncClient::append(const std::string &key, const std::string &value, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->append(key, value, callback);
}

void TairClusterAsyncClient::bitcount(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->bitcount(key, callback);
}

void TairClusterAsyncClient::bitcount(const std::string &key, const BitPositonParams &params, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->bitcount(key, params, callback);
}

void TairClusterAsyncClient::bitfield(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->bitfield(key, args, callback);
}

void TairClusterAsyncClient::bitfieldRo(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->bitfieldRo(key, args, callback);
}

void TairClusterAsyncClient::bitop(const BitOperation &op, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(TairResult<int64_t>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(*keys.begin());
    client->bitop(op, keys, callback);
}

void TairClusterAsyncClient::bitpos(const std::string &key, int64_t bit, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->bitpos(key, bit, callback);
}

void TairClusterAsyncClient::bitpos(const std::string &key, int64_t bit, const BitPositonParams &params, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->bitpos(key, bit, params, callback);
}

void TairClusterAsyncClient::setbit(const std::string &key, int64_t offset, int64_t value, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->setbit(key, offset, value, callback);
}

void TairClusterAsyncClient::getbit(const std::string &key, int64_t offset, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->getbit(key, offset, callback);
}

void TairClusterAsyncClient::decr(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->decr(key, callback);
}

void TairClusterAsyncClient::decrby(const std::string &key, int64_t decrement, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->decrby(key, decrement, callback);
}

void TairClusterAsyncClient::getrange(const std::string &key, int64_t start, int64_t end, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->getrange(key, start, end, callback);
}

void TairClusterAsyncClient::getset(const std::string &key, const std::string &value, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->getset(key, value, callback);
}

void TairClusterAsyncClient::set(const std::string &key, const std::string &value, const SetParams &params, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->set(key, value, params, callback);
}

void TairClusterAsyncClient::set(const std::string &key, const std::string &value, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->set(key, value, callback);
}

void TairClusterAsyncClient::get(const std::string &key, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->get(key, callback);
}

void TairClusterAsyncClient::incr(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->incr(key, callback);
}

void TairClusterAsyncClient::incrby(const std::string &key, int64_t increment, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->incrby(key, increment, callback);
}

void TairClusterAsyncClient::incrbyfloat(const std::string &key, double increment, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->incrbyfloat(key, increment, callback);
}

void TairClusterAsyncClient::mget(InitializerList<std::string> keys, const ResultVectorStringPtrCallback &callback) {
    callback(TairResult<std::vector<std::shared_ptr<std::string>>>::createErr(E_CLUSTER_NOT_SUPPORT_COMMAND));
}

void TairClusterAsyncClient::mset(InitializerList<std::string> kvs, const ResultStringCallback &callback) {
    callback(TairResult<std::string>::createErr(E_CLUSTER_NOT_SUPPORT_COMMAND));
}

void TairClusterAsyncClient::msetnx(InitializerList<std::string> kvs, const ResultIntegerCallback &callback) {
    callback(TairResult<int64_t>::createErr(E_CLUSTER_NOT_SUPPORT_COMMAND));
}

void TairClusterAsyncClient::psetex(const std::string &key, int64_t milliseconds, const std::string &value, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->psetex(key, milliseconds, value, callback);
}

void TairClusterAsyncClient::setex(const std::string &key, int64_t seconds, const std::string &value, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->setex(key, seconds, value, callback);
}

void TairClusterAsyncClient::setnx(const std::string &key, const std::string &value, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->setnx(key, value, callback);
}

void TairClusterAsyncClient::setrange(const std::string &key, int64_t offset, const std::string &value, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->setrange(key, offset, value, callback);
}

void TairClusterAsyncClient::strlen(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->strlen(key, callback);
}

void TairClusterAsyncClient::getdel(const std::string &key, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->getdel(key, callback);
}

void TairClusterAsyncClient::getex(const std::string &key, const GetExParams &params, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->getex(key, params, callback);
}

// -------------------------------- List Command --------------------------------
void TairClusterAsyncClient::blpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(*keys.begin());
    client->blpop(keys, timeout, callback);
}

void TairClusterAsyncClient::brpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(*keys.begin());
    client->brpop(keys, timeout, callback);
}

void TairClusterAsyncClient::brpoplpush(const std::string &src, const std::string &dest, int64_t timeout, const ResultStringPtrCallback &callback) {
    if (!checkKeyInSameSlot({src, dest})) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(src);
    client->brpoplpush(src, dest, timeout, callback);
}

void TairClusterAsyncClient::lindex(const std::string &key, int64_t index, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->lindex(key, index, callback);
}

void TairClusterAsyncClient::linsert(const std::string &key, const ListDirection &direction, const std::string &pivot, const std::string &element, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->linsert(key, direction, pivot, element, callback);
}

void TairClusterAsyncClient::llen(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->llen(key, callback);
}

void TairClusterAsyncClient::lpop(const std::string &key, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->lpop(key, callback);
}

void TairClusterAsyncClient::lpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->lpop(key, count, callback);
}

void TairClusterAsyncClient::lpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->lpush(key, elements, callback);
}

void TairClusterAsyncClient::lpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->lpushx(key, elements, callback);
}

void TairClusterAsyncClient::lrange(const std::string &key, int64_t start, int64_t stop, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->lrange(key, start, stop, callback);
}

void TairClusterAsyncClient::lrem(const std::string &key, int64_t count, const std::string &element, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->lrem(key, count, element, callback);
}

void TairClusterAsyncClient::lset(const std::string &key, int64_t index, const std::string &element, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->lset(key, index, element, callback);
}

void TairClusterAsyncClient::ltrim(const std::string &key, int64_t start, int64_t stop, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->ltrim(key, start, stop, callback);
}

void TairClusterAsyncClient::rpop(const std::string &key, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->rpop(key, callback);
}

void TairClusterAsyncClient::rpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->rpop(key, count, callback);
}

void TairClusterAsyncClient::rpoplpush(const std::string &src, const std::string &dest, const ResultStringPtrCallback &callback) {
    if (!checkKeyInSameSlot({src, dest})) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(src);
    client->rpoplpush(src, dest, callback);
}

void TairClusterAsyncClient::rpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->rpush(key, elements, callback);
}

void TairClusterAsyncClient::rpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->rpushx(key, elements, callback);
}

void TairClusterAsyncClient::lmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, const ResultStringPtrCallback &callback) {
    if (!checkKeyInSameSlot({src, dest})) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(src);
    client->lmove(src, dest, ld, rd, callback);
}

void TairClusterAsyncClient::blmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, int64_t timeout, const ResultStringPtrCallback &callback) {
    if (!checkKeyInSameSlot({src, dest})) {
        callback(TairResult<std::shared_ptr<std::string>>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(src);
    client->blmove(src, dest, ld, rd, timeout, callback);
}

// -------------------------------- Set Command --------------------------------
void TairClusterAsyncClient::sadd(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->sadd(key, members, callback);
}

void TairClusterAsyncClient::scard(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->scard(key, callback);
}

void TairClusterAsyncClient::sismember(const std::string &key, const std::string &member, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->sismember(key, member, callback);
}

void TairClusterAsyncClient::smembers(const std::string &key, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->smembers(key, callback);
}

void TairClusterAsyncClient::smove(const std::string &src, const std::string &dest, const std::string &member, const ResultIntegerCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::srem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->srem(key, members, callback);
}

void TairClusterAsyncClient::spop(const std::string &key, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->spop(key, callback);
}

void TairClusterAsyncClient::spop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->spop(key, count, callback);
}

void TairClusterAsyncClient::srandmember(const std::string &key, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->srandmember(key, callback);
}

void TairClusterAsyncClient::srandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->srandmember(key, count, callback);
}

void TairClusterAsyncClient::sdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::sinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::sunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::sdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::sinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::sunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::sscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) {
    auto client = getClientByKey(key);
    client->sscan(key, cursor, callback);
}

void TairClusterAsyncClient::sscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    auto client = getClientByKey(key);
    client->sscan(key, cursor, params, callback);
}

void TairClusterAsyncClient::smismember(const std::string &key, InitializerList<std::string> members, const ResultVectorIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->smismember(key, members, callback);
}

// -------------------------------- Hash Command --------------------------------
void TairClusterAsyncClient::hdel(const std::string &key, InitializerList<std::string> fields, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->hdel(key, fields, callback);
}

void TairClusterAsyncClient::hexists(const std::string &key, const std::string &field, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->hexists(key, field, callback);
}

void TairClusterAsyncClient::hget(const std::string &key, const std::string &field, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->hget(key, field, callback);
}

void TairClusterAsyncClient::hgetall(const std::string &key, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->hgetall(key, callback);
}

void TairClusterAsyncClient::hincrby(const std::string &key, const std::string &field, int64_t increment, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->hincrby(key, field, increment, callback);
}

void TairClusterAsyncClient::hincrbyfloat(const std::string &key, const std::string &field, double increment, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->hincrbyfloat(key, field, increment, callback);
}

void TairClusterAsyncClient::hkeys(const std::string &key, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->hkeys(key, callback);
}

void TairClusterAsyncClient::hvals(const std::string &key, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->hvals(key, callback);
}

void TairClusterAsyncClient::hlen(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->hlen(key, callback);
}

void TairClusterAsyncClient::hmget(const std::string &key, InitializerList<std::string> fields, const ResultVectorStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->hmget(key, fields, callback);
}

void TairClusterAsyncClient::hset(const std::string &key, const std::string &field, const std::string &value, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->hset(key, field, value, callback);
}

void TairClusterAsyncClient::hset(const std::string &key, InitializerList<std::string> kvs, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->hset(key, kvs, callback);
}

void TairClusterAsyncClient::hsetnx(const std::string &key, const std::string &field, const std::string &value, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->hsetnx(key, field, value, callback);
}

void TairClusterAsyncClient::hstrlen(const std::string &key, const std::string &field, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->hstrlen(key, field, callback);
}

void TairClusterAsyncClient::hscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) {
    auto client = getClientByKey(key);
    client->hscan(key, cursor, callback);
}

void TairClusterAsyncClient::hscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    auto client = getClientByKey(key);
    client->hscan(key, cursor, params, callback);
}

void TairClusterAsyncClient::hrandfield(const std::string &key, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->hrandfield(key, callback);
}

void TairClusterAsyncClient::hrandfield(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->hrandfield(key, count, callback);
}

// -------------------------------- Zset Command --------------------------------
void TairClusterAsyncClient::zadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->zadd(key, elements, callback);
}

void TairClusterAsyncClient::zadd(const std::string &key, const ZAddParams &params, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->zadd(key, params, elements, callback);
}

void TairClusterAsyncClient::zincrby(const std::string &key, int64_t increment, const std::string &member, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->zincrby(key, increment, member, callback);
}

void TairClusterAsyncClient::zscore(const std::string &key, const std::string &member, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->zscore(key, member, callback);
}

void TairClusterAsyncClient::zrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->zrank(key, member, callback);
}

void TairClusterAsyncClient::zrevrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->zrevrank(key, member, callback);
}

void TairClusterAsyncClient::zcard(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->zcard(key, callback);
}

void TairClusterAsyncClient::zcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->zcount(key, min, max, callback);
}

void TairClusterAsyncClient::zlexcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->zlexcount(key, min, max, callback);
}

void TairClusterAsyncClient::zpopmax(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->zpopmax(key, count, callback);
}

void TairClusterAsyncClient::zpopmin(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->zpopmin(key, count, callback);
}

void TairClusterAsyncClient::bzpopmax(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(*keys.begin());
    client->bzpopmax(keys, timeout, callback);
}

void TairClusterAsyncClient::bzpopmin(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(TairResult<std::vector<std::string>>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(*keys.begin());
    client->bzpopmin(keys, timeout, callback);
}

void TairClusterAsyncClient::zrange(const std::string &key, const std::string &start, const std::string &stop, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->zrange(key, start, stop, callback);
}

void TairClusterAsyncClient::zrange(const std::string &key, const std::string &start, const std::string &stop, const ZRangeParams &params, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->zrange(key, start, stop, params, callback);
}

void TairClusterAsyncClient::zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) {
    if (!checkKeyInSameSlot({dest, src})) {
        callback(TairResult<int64_t>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(dest);
    client->zrangestore(dest, src, min, max, callback);
}

void TairClusterAsyncClient::zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ZRangeParams &params, const ResultIntegerCallback &callback) {
    if (!checkKeyInSameSlot({dest, src})) {
        callback(TairResult<int64_t>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(dest);
    client->zrangestore(dest, src, min, max, params, callback);
}

void TairClusterAsyncClient::zrem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->zrem(key, members, callback);
}

void TairClusterAsyncClient::zremrange(const ZRemRangeOption &option, const std::string &key, const std::string &begin, const std::string &end, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->zremrange(option, key, begin, end, callback);
}

void TairClusterAsyncClient::zinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zinter(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zinterstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zunion(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zunionstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zdiffWithScores(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zdiffstoreWithScores(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    // TODO
}

void TairClusterAsyncClient::zmscore(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->zmscore(key, members, callback);
}

void TairClusterAsyncClient::zrandmember(const std::string &key, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->zrandmember(key, callback);
}

void TairClusterAsyncClient::zrandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->zrandmember(key, count, callback);
}

void TairClusterAsyncClient::zscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) {
    auto client = getClientByKey(key);
    client->zscan(key, cursor, callback);
}

void TairClusterAsyncClient::zscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) {
    auto client = getClientByKey(key);
    client->zscan(key, cursor, params, callback);
}

// -------------------------------- HyperLogLog Command --------------------------------
void TairClusterAsyncClient::pfadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->pfadd(key, elements, callback);
}

void TairClusterAsyncClient::pfcount(InitializerList<std::string> keys, const ResultIntegerCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(TairResult<int64_t>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(*keys.begin());
    client->pfcount(keys, callback);
}

void TairClusterAsyncClient::pfmerge(const std::string &dest, InitializerList<std::string> keys, const ResultStringCallback &callback) {
    if (!checkKeyInSameSlot(dest, keys)) {
        callback(TairResult<std::string>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(dest);
    client->pfmerge(dest, keys, callback);
}

// -------------------------------- Geo Command --------------------------------
void TairClusterAsyncClient::geoadd(const std::string &key, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->geoadd(key, members, callback);
}

void TairClusterAsyncClient::geoadd(const std::string &key, const GeoAddParams &params, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->geoadd(key, params, members, callback);
}

void TairClusterAsyncClient::geodist(const std::string &key, const std::string &member1, const std::string &member2, const GeoUnit &unit, const ResultStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->geodist(key, member1, member2, unit, callback);
}

void TairClusterAsyncClient::geohash(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback) {
    auto client = getClientByKey(key);
    client->geohash(key, members, callback);
}

void TairClusterAsyncClient::geopos(const std::string &key, InitializerList<std::string> members, const ResultGeoposCallback &callback) {
    auto client = getClientByKey(key);
    client->geopos(key, members, callback);
}

void TairClusterAsyncClient::georadius(const std::string &key, double longitude, double latitude, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->georadius(key, longitude, latitude, radius, unit, callback);
}

void TairClusterAsyncClient::georadiusbymember(const std::string &key, const std::string &member, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback) {
    auto client = getClientByKey(key);
    client->georadiusbymember(key, member, radius, unit, callback);
}

// -------------------------------- Stream Command --------------------------------
void TairClusterAsyncClient::xadd(const std::string &key, const std::string &id, InitializerList<std::string> elements, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->xadd(key, id, elements, callback);
}

void TairClusterAsyncClient::xlen(const std::string &key, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->xlen(key, callback);
}

void TairClusterAsyncClient::xdel(const std::string &key, InitializerList<std::string> ids, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->xdel(key, ids, callback);
}

void TairClusterAsyncClient::xack(const std::string &key, const std::string &group, InitializerList<std::string> ids, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->xack(key, group, ids, callback);
}

void TairClusterAsyncClient::xgroupCreate(const std::string &key, const std::string &group, const std::string &id, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->xgroupCreate(key, group, id, callback);
}

void TairClusterAsyncClient::xgroupCreateConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->xgroupCreateConsumer(key, group, consumer, callback);
}

void TairClusterAsyncClient::xgroupDelConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->xgroupDelConsumer(key, group, consumer, callback);
}

void TairClusterAsyncClient::xgroupDestroy(const std::string &key, const std::string &group, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->xgroupDestroy(key, group, callback);
}

void TairClusterAsyncClient::xgroupSetID(const std::string &key, const std::string &group, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->xgroupSetID(key, group, callback);
}

void TairClusterAsyncClient::xpending(const std::string &key, const std::string &group, const ResultXPendingCallback &callback) {
    auto client = getClientByKey(key);
    client->xpending(key, group, callback);
}

void TairClusterAsyncClient::xrange(const std::string &key, const std::string &start, const std::string &end, const ResultXRangeCallback &callback) {
    auto client = getClientByKey(key);
    client->xrange(key, start, end, callback);
}

void TairClusterAsyncClient::xrevrange(const std::string &key, const std::string &end, const std::string &start, const ResultXRangeCallback &callback) {
    auto client = getClientByKey(key);
    client->xrevrange(key, end, start, callback);
}

void TairClusterAsyncClient::xread(int64_t count, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(TairResult<std::vector<XReadResult>>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(*keys.begin());
    client->xread(count, keys, ids, callback);
}

void TairClusterAsyncClient::xreadgroup(const std::string &group, const std::string &consumer, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(TairResult<std::vector<XReadResult>>::createErr(E_NOT_IN_SAME_SLOT));
        return;
    }
    auto client = getClientByKey(*keys.begin());
    client->xreadgroup(group, consumer, keys, ids, callback);
}

void TairClusterAsyncClient::xtrim(const std::string &key, const std::string &strategy, int64_t threshold, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(key);
    client->xtrim(key, strategy, threshold, callback);
}

void TairClusterAsyncClient::xsetid(const std::string &key, const std::string &last_id, const ResultStringCallback &callback) {
    auto client = getClientByKey(key);
    client->xsetid(key, last_id, callback);
}

// -------------------------------- Script Command --------------------------------
void TairClusterAsyncClient::eval(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(nullptr, nullptr, nullptr);
        return;
    }
    auto client = getClientRandom();
    client->eval(script, keys, args, callback);
}

void TairClusterAsyncClient::evalRo(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(nullptr, nullptr, nullptr);
        return;
    }
    auto client = getClientRandom();
    client->evalRo(script, keys, args, callback);
}

void TairClusterAsyncClient::evalsha(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(nullptr, nullptr, nullptr);
        return;
    }
    auto client = getClientRandom();
    client->evalsha(sha1, keys, args, callback);
}

void TairClusterAsyncClient::evalshaRo(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) {
    if (!checkKeyInSameSlot(keys)) {
        callback(nullptr, nullptr, nullptr);
        return;
    }
    auto client = getClientRandom();
    client->evalshaRo(sha1, keys, args, callback);
}

void TairClusterAsyncClient::scriptLoad(const std::string &script, const ResultStringCallback &callback) {
    bool all_node_ok = true;
    for (const auto &n : client_map_) {
        auto server_addr = n.first;
        auto clientPtr = n.second;
        clientPtr->scriptLoad(script, [&server_addr, &all_node_ok](const TairResult<std::string> &result) {
            if (!result.isSuccess()) {
                all_node_ok = false;
                LOG_ERROR("cluster call script load failed, node: {}, err: {}", server_addr, result.getErr());
            }
        });
    }
    if (all_node_ok) {
        callback(TairResult<std::string>::create("ok"));
    } else {
        callback(TairResult<std::string>::createErr("script load failed"));
    }
}

void TairClusterAsyncClient::scriptFlush(const ResultStringCallback &callback) {
    bool all_node_ok = true;
    for (const auto &n : client_map_) {
        auto server_addr = n.first;
        auto clientPtr = n.second;
        clientPtr->scriptFlush([&server_addr, &all_node_ok](const TairResult<std::string> &result) {
            if (!result.isSuccess()) {
                all_node_ok = false;
                LOG_ERROR("cluster call script flush failed, node: {}, err: {}", server_addr, result.getErr());
            }
        });
    }
    if (all_node_ok) {
        callback(TairResult<std::string>::create("ok"));
    } else {
        callback(TairResult<std::string>::createErr("script flush failed"));
    }
}

void TairClusterAsyncClient::scriptKill(const ResultStringCallback &callback) {
    bool all_node_ok = true;
    for (const auto &n : client_map_) {
        auto server_addr = n.first;
        auto clientPtr = n.second;
        clientPtr->scriptKill([&server_addr, &all_node_ok](const TairResult<std::string> &result) {
            if (!result.isSuccess()) {
                all_node_ok = false;
                LOG_ERROR("cluster call script kill failed, node: {}, err: {}", server_addr, result.getErr());
            }
        });
    }
    if (all_node_ok) {
        callback(TairResult<std::string>::create("ok"));
    } else {
        callback(TairResult<std::string>::createErr("script kill failed"));
    }
}

void TairClusterAsyncClient::scriptExists(InitializerList<std::string> sha1s, const ResultVectorIntegerCallback &callback) {
    auto client = getClientRandom();
    client->scriptExists(sha1s, callback);
}

// -------------------------------- Connection Command --------------------------------
void TairClusterAsyncClient::auth(const std::string &password, const ResultStringCallback &callback) {
    auto client = getClientRandom();
    client->auth(password, callback);
}

void TairClusterAsyncClient::auth(const std::string &user, const std::string &password, const ResultStringCallback &callback) {
    auto client = getClientRandom();
    client->auth(user, password, callback);
}

void TairClusterAsyncClient::quit(const ResultStringCallback &callback) {
    for (const auto &n : client_map_) {
        auto server_addr = n.first;
        auto clientPtr = n.second;
        clientPtr->quit([](const TairResult<std::string> &result) {
            // ignore result whether ok or fail
        });
    }
    callback(TairResult<std::string>::create("ok"));
}

// -------------------------------- Server Command --------------------------------
void TairClusterAsyncClient::flushall(const ResultStringCallback &callback) {
    bool all_node_flush_ok = true;
    for (const auto &n : client_map_) {
        auto server_addr = n.first;
        auto clientPtr = n.second;
        clientPtr->flushall([&server_addr, &all_node_flush_ok](const TairResult<std::string> &result) {
            if (!result.isSuccess()) {
                all_node_flush_ok = false;
                LOG_ERROR("cluster call flushall failed, node: {}, err: {}", server_addr, result.getErr());
            }
        });
    }
    if (all_node_flush_ok) {
        callback(TairResult<std::string>::create("ok"));
    } else {
        callback(TairResult<std::string>::createErr("flushall failed"));
    }
}

// -------------------------------- Pubsub Command --------------------------------
void TairClusterAsyncClient::publish(const std::string &channel, const std::string &message, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(channel);
    client->publish(channel, message, callback);
}

void TairClusterAsyncClient::clusterPublish(const std::string &channel, const std::string &message,
                                            const std::string &name, int flag, const ResultIntegerCallback &callback) {
    auto client = getClientByKey(channel);
    client->clusterPublish(channel, message, name, flag, callback);
}

// -------------------------------- Cluster Command --------------------------------
void TairClusterAsyncClient::clusterNodes(const ResultStringCallback &callback) {
    auto client = getClientRandom();
    client->clusterNodes(callback);
}

} // namespace tair::client
