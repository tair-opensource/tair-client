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
#include "client/TairClientWrapper.hpp"

#include "client/TairClient.hpp"

namespace tair::client {

#define FUTURE_CALL(TYPE, FUNC_NAME, ...)                                 \
    auto promise = std::make_shared<std::promise<TYPE>>();                \
    client_.FUNC_NAME(__VA_ARGS__ __VA_OPT__(, )[promise](auto &result) { \
        promise->set_value(result);                                       \
    });                                                                   \
    return promise->get_future()

// -------------------------------- send Command --------------------------------
std::future<PacketPtr> TairClientWrapper::sendCommand(CommandArgv &&argv) {
    auto promise = std::make_shared<std::promise<PacketPtr>>();
    client_.sendCommand(std::move(argv), [promise](auto *, auto &, auto &resp) { promise->set_value(resp); });
    return promise->get_future();
}

std::future<PacketPtr> TairClientWrapper::sendCommand(const CommandArgv &argv) {
    auto promise = std::make_shared<std::promise<PacketPtr>>();
    client_.sendCommand(argv, [promise](auto *, auto &, auto &result) { promise->set_value(result); });
    return promise->get_future();
}

// -------------------------------- Generic Command --------------------------------
std::future<TairResult<int64_t>> TairClientWrapper::del(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, del, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::del(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, del, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::unlink(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, unlink, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::unlink(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, unlink, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::exists(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, exists, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::exists(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, exists, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::expire(const std::string &key, int64_t timeout) {
    FUTURE_CALL(TairResult<int64_t>, expire, key, timeout);
}

std::future<TairResult<int64_t>> TairClientWrapper::expire(const std::string &key, int64_t timeout, const ExpireParams &params) {
    FUTURE_CALL(TairResult<int64_t>, expire, key, timeout, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::expireat(const std::string &key, int64_t timestamp) {
    FUTURE_CALL(TairResult<int64_t>, expireat, key, timestamp);
}

std::future<TairResult<int64_t>> TairClientWrapper::expireat(const std::string &key, int64_t timestamp, const ExpireParams &params) {
    FUTURE_CALL(TairResult<int64_t>, expireat, key, timestamp, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::persist(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, persist, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::pexpire(const std::string &key, int64_t timeout) {
    FUTURE_CALL(TairResult<int64_t>, pexpire, key, timeout);
}

std::future<TairResult<int64_t>> TairClientWrapper::pexpire(const std::string &key, int64_t timeout, const ExpireParams &params) {
    FUTURE_CALL(TairResult<int64_t>, pexpire, key, timeout, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::pexpireat(const std::string &key, int64_t timestamp) {
    FUTURE_CALL(TairResult<int64_t>, pexpireat, key, timestamp);
}

std::future<TairResult<int64_t>> TairClientWrapper::pexpireat(const std::string &key, int64_t timestamp, const ExpireParams &params) {
    FUTURE_CALL(TairResult<int64_t>, pexpireat, key, timestamp, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::ttl(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, ttl, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::pttl(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, pttl, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::touch(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, touch, keys);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::dump(const std::string &key) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, dump, key);
}

std::future<TairResult<std::string>> TairClientWrapper::restore(const std::string &key, int64_t ttl, const std::string &value) {
    FUTURE_CALL(TairResult<std::string>, restore, key, ttl, value);
}

std::future<TairResult<std::string>> TairClientWrapper::restore(const std::string &key, int64_t ttl, const std::string &value, const RestoreParams &params) {
    FUTURE_CALL(TairResult<std::string>, restore, key, ttl, value, params);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::keys(const std::string &pattern) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, keys, pattern);
}

std::future<TairResult<int64_t>> TairClientWrapper::move(const std::string &key, int64_t db) {
    FUTURE_CALL(TairResult<int64_t>, move, key, db);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::randomkey() {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, randomkey);
}

std::future<TairResult<std::string>> TairClientWrapper::rename(const std::string &key, const std::string &newkey) {
    FUTURE_CALL(TairResult<std::string>, rename, key, newkey);
}

std::future<TairResult<int64_t>> TairClientWrapper::renamenx(const std::string &key, const std::string &newkey) {
    FUTURE_CALL(TairResult<int64_t>, renamenx, key, newkey);
}

std::future<TairResult<std::string>> TairClientWrapper::type(const std::string &key) {
    FUTURE_CALL(TairResult<std::string>, type, key);
}

std::future<TairResult<ScanResult>> TairClientWrapper::scan(const std::string &cursor) {
    FUTURE_CALL(TairResult<ScanResult>, scan, cursor);
}

std::future<TairResult<ScanResult>> TairClientWrapper::scan(const std::string &cursor, const ScanParams &params) {
    FUTURE_CALL(TairResult<ScanResult>, scan, cursor, params);
}

std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> TairClientWrapper::sort(const std::string &key) {
    FUTURE_CALL(TairResult<std::vector<std::shared_ptr<std::string>>>, sort, key);
}

std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> TairClientWrapper::sort(const std::string &key, const SortParams &params) {
    FUTURE_CALL(TairResult<std::vector<std::shared_ptr<std::string>>>, sort, key, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::sortStore(const std::string &key, const std::string &storekey) {
    FUTURE_CALL(TairResult<int64_t>, sortStore, key, storekey);
}

std::future<TairResult<int64_t>> TairClientWrapper::sortStore(const std::string &key, const std::string &storekey, const SortParams &params) {
    FUTURE_CALL(TairResult<int64_t>, sortStore, key, storekey, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::copy(const std::string &key, const std::string &destkey) {
    FUTURE_CALL(TairResult<int64_t>, copy, key, destkey);
}

std::future<TairResult<int64_t>> TairClientWrapper::copy(const std::string &key, const std::string &destkey, const CopyParams &params) {
    FUTURE_CALL(TairResult<int64_t>, copy, key, destkey, params);
}

// -------------------------------- String Command --------------------------------
std::future<TairResult<int64_t>> TairClientWrapper::append(const std::string &key, const std::string &value) {
    FUTURE_CALL(TairResult<int64_t>, append, key, value);
}

std::future<TairResult<int64_t>> TairClientWrapper::bitcount(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, bitcount, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::bitcount(const std::string &key, const BitPositonParams &params) {
    FUTURE_CALL(TairResult<int64_t>, bitcount, key, params);
}

std::future<TairResult<std::vector<std::shared_ptr<int64_t>>>> TairClientWrapper::bitfield(const std::string &key, InitializerList<std::string> args) {
    FUTURE_CALL(TairResult<std::vector<std::shared_ptr<int64_t>>>, bitfield, key, args);
}

std::future<TairResult<std::vector<std::shared_ptr<int64_t>>>> TairClientWrapper::bitfieldRo(const std::string &key, InitializerList<std::string> args) {
    FUTURE_CALL(TairResult<std::vector<std::shared_ptr<int64_t>>>, bitfieldRo, key, args);
}

std::future<TairResult<int64_t>> TairClientWrapper::bitop(const BitOperation &op, InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, bitop, op, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::bitpos(const std::string &key, int64_t bit) {
    FUTURE_CALL(TairResult<int64_t>, bitpos, key, bit);
}

std::future<TairResult<int64_t>> TairClientWrapper::bitpos(const std::string &key, int64_t bit, const BitPositonParams &params) {
    FUTURE_CALL(TairResult<int64_t>, bitpos, key, bit, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::setbit(const std::string &key, int64_t offset, int64_t value) {
    FUTURE_CALL(TairResult<int64_t>, setbit, key, offset, value);
}

std::future<TairResult<int64_t>> TairClientWrapper::getbit(const std::string &key, int64_t offset) {
    FUTURE_CALL(TairResult<int64_t>, getbit, key, offset);
}

std::future<TairResult<int64_t>> TairClientWrapper::decr(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, decr, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::decrby(const std::string &key, int64_t decrement) {
    FUTURE_CALL(TairResult<int64_t>, decrby, key, decrement);
}

std::future<TairResult<std::string>> TairClientWrapper::getrange(const std::string &key, int64_t start, int64_t end) {
    FUTURE_CALL(TairResult<std::string>, getrange, key, start, end);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::getset(const std::string &key, const std::string &value) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, getset, key, value);
}

std::future<TairResult<std::string>> TairClientWrapper::set(const std::string &key, const std::string &value, const SetParams &params) {
    FUTURE_CALL(TairResult<std::string>, set, key, value, params);
}

std::future<TairResult<std::string>> TairClientWrapper::set(const std::string &key, const std::string &value) {
    FUTURE_CALL(TairResult<std::string>, set, key, value);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::get(const std::string &key) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, get, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::incr(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, incr, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::incrby(const std::string &key, int64_t increment) {
    FUTURE_CALL(TairResult<int64_t>, incrby, key, increment);
}

std::future<TairResult<std::string>> TairClientWrapper::incrbyfloat(const std::string &key, double increment) {
    FUTURE_CALL(TairResult<std::string>, incrbyfloat, key, increment);
}

std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> TairClientWrapper::mget(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<std::vector<std::shared_ptr<std::string>>>, mget, keys);
}

std::future<TairResult<std::string>> TairClientWrapper::mset(InitializerList<std::string> kvs) {
    FUTURE_CALL(TairResult<std::string>, mset, kvs);
}

std::future<TairResult<int64_t>> TairClientWrapper::msetnx(InitializerList<std::string> kvs) {
    FUTURE_CALL(TairResult<int64_t>, msetnx, kvs);
}

std::future<TairResult<std::string>> TairClientWrapper::psetex(const std::string &key, int64_t milliseconds, const std::string &value) {
    FUTURE_CALL(TairResult<std::string>, psetex, key, milliseconds, value);
}

std::future<TairResult<std::string>> TairClientWrapper::setex(const std::string &key, int64_t seconds, const std::string &value) {
    FUTURE_CALL(TairResult<std::string>, setex, key, seconds, value);
}

std::future<TairResult<int64_t>> TairClientWrapper::setnx(const std::string &key, const std::string &value) {
    FUTURE_CALL(TairResult<int64_t>, setnx, key, value);
}

std::future<TairResult<int64_t>> TairClientWrapper::setrange(const std::string &key, int64_t offset, const std::string &value) {
    FUTURE_CALL(TairResult<int64_t>, setrange, key, offset, value);
}

std::future<TairResult<int64_t>> TairClientWrapper::strlen(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, strlen, key);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::getdel(const std::string &key) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, getdel, key);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::getex(const std::string &key, const GetExParams &params) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, getex, key, params);
}

// -------------------------------- List Command --------------------------------
std::future<TairResult<std::vector<std::string>>> TairClientWrapper::blpop(InitializerList<std::string> keys, int64_t timeout) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, blpop, keys, timeout);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::brpop(InitializerList<std::string> keys, int64_t timeout) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, brpop, keys, timeout);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::brpoplpush(const std::string &src, const std::string &dest, int64_t timeout) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, brpoplpush, src, dest, timeout);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::lindex(const std::string &key, int64_t index) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, lindex, key, index);
}

std::future<TairResult<int64_t>> TairClientWrapper::linsert(const std::string &key, const ListDirection &direction, const std::string &pivot, const std::string &element) {
    FUTURE_CALL(TairResult<int64_t>, linsert, key, direction, pivot, element);
}

std::future<TairResult<int64_t>> TairClientWrapper::llen(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, llen, key);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::lpop(const std::string &key) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, lpop, key);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::lpop(const std::string &key, int64_t count) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, lpop, key, count);
}

std::future<TairResult<int64_t>> TairClientWrapper::lpush(const std::string &key, InitializerList<std::string> elements) {
    FUTURE_CALL(TairResult<int64_t>, lpush, key, elements);
}

std::future<TairResult<int64_t>> TairClientWrapper::lpushx(const std::string &key, InitializerList<std::string> elements) {
    FUTURE_CALL(TairResult<int64_t>, lpushx, key, elements);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::lrange(const std::string &key, int64_t start, int64_t stop) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, lrange, key, start, stop);
}

std::future<TairResult<int64_t>> TairClientWrapper::lrem(const std::string &key, int64_t count, const std::string &element) {
    FUTURE_CALL(TairResult<int64_t>, lrem, key, count, element);
}

std::future<TairResult<std::string>> TairClientWrapper::lset(const std::string &key, int64_t index, const std::string &element) {
    FUTURE_CALL(TairResult<std::string>, lset, key, index, element);
}

std::future<TairResult<std::string>> TairClientWrapper::ltrim(const std::string &key, int64_t start, int64_t stop) {
    FUTURE_CALL(TairResult<std::string>, ltrim, key, start, stop);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::rpop(const std::string &key) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, rpop, key);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::rpop(const std::string &key, int64_t count) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, rpop, key, count);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::rpoplpush(const std::string &src, const std::string &dest) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, rpoplpush, src, dest);
}

std::future<TairResult<int64_t>> TairClientWrapper::rpush(const std::string &key, InitializerList<std::string> elements) {
    FUTURE_CALL(TairResult<int64_t>, rpush, key, elements);
}

std::future<TairResult<int64_t>> TairClientWrapper::rpushx(const std::string &key, InitializerList<std::string> elements) {
    FUTURE_CALL(TairResult<int64_t>, rpushx, key, elements);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::lmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, lmove, src, dest, ld, rd);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::blmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, int64_t timeout) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, blmove, src, dest, ld, rd, timeout);
}

// -------------------------------- Set Command --------------------------------
std::future<TairResult<int64_t>> TairClientWrapper::sadd(const std::string &key, InitializerList<std::string> members) {
    FUTURE_CALL(TairResult<int64_t>, sadd, key, members);
}

std::future<TairResult<int64_t>> TairClientWrapper::scard(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, scard, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::sismember(const std::string &key, const std::string &member) {
    FUTURE_CALL(TairResult<int64_t>, sismember, key, member);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::smembers(const std::string &key) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, smembers, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::smove(const std::string &src, const std::string &dest, const std::string &member) {
    FUTURE_CALL(TairResult<int64_t>, smove, src, dest, member);
}

std::future<TairResult<int64_t>> TairClientWrapper::srem(const std::string &key, InitializerList<std::string> members) {
    FUTURE_CALL(TairResult<int64_t>, srem, key, members);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::spop(const std::string &key) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, spop, key);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::spop(const std::string &key, int64_t count) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, spop, key, count);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::srandmember(const std::string &key) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, srandmember, key);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::srandmember(const std::string &key, int64_t count) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, srandmember, key, count);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::sdiff(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, sdiff, keys);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::sinter(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, sinter, keys);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::sunion(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, sunion, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::sdiffstore(const std::string &dest, InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, sdiffstore, dest, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::sinterstore(const std::string &dest, InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, sinterstore, dest, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::sunionstore(const std::string &dest, InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, sunionstore, dest, keys);
}

std::future<TairResult<ScanResult>> TairClientWrapper::sscan(const std::string &key, const std::string &cursor) {
    FUTURE_CALL(TairResult<ScanResult>, sscan, key, cursor);
}

std::future<TairResult<ScanResult>> TairClientWrapper::sscan(const std::string &key, const std::string &cursor, const ScanParams &params) {
    FUTURE_CALL(TairResult<ScanResult>, sscan, key, cursor, params);
}

std::future<TairResult<std::vector<int64_t>>> TairClientWrapper::smismember(const std::string &key, InitializerList<std::string> members) {
    FUTURE_CALL(TairResult<std::vector<int64_t>>, smismember, key, members);
}

// -------------------------------- Hash Command --------------------------------
std::future<TairResult<int64_t>> TairClientWrapper::hdel(const std::string &key, InitializerList<std::string> fields) {
    FUTURE_CALL(TairResult<int64_t>, hdel, key, fields);
}

std::future<TairResult<int64_t>> TairClientWrapper::hexists(const std::string &key, const std::string &field) {
    FUTURE_CALL(TairResult<int64_t>, hexists, key, field);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::hget(const std::string &key, const std::string &field) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, hget, key, field);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::hgetall(const std::string &key) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, hgetall, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::hincrby(const std::string &key, const std::string &field, int64_t increment) {
    FUTURE_CALL(TairResult<int64_t>, hincrby, key, field, increment);
}

std::future<TairResult<std::string>> TairClientWrapper::hincrbyfloat(const std::string &key, const std::string &field, double increment) {
    FUTURE_CALL(TairResult<std::string>, hincrbyfloat, key, field, increment);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::hkeys(const std::string &key) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, hkeys, key);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::hvals(const std::string &key) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, hvals, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::hlen(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, hlen, key);
}

std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> TairClientWrapper::hmget(const std::string &key, InitializerList<std::string> fields) {
    FUTURE_CALL(TairResult<std::vector<std::shared_ptr<std::string>>>, hmget, key, fields);
}

std::future<TairResult<int64_t>> TairClientWrapper::hset(const std::string &key, const std::string &filed, const std::string &value) {
    FUTURE_CALL(TairResult<int64_t>, hset, key, filed, value);
}

std::future<TairResult<int64_t>> TairClientWrapper::hset(const std::string &key, InitializerList<std::string> kvs) {
    FUTURE_CALL(TairResult<int64_t>, hset, key, kvs);
}

std::future<TairResult<int64_t>> TairClientWrapper::hsetnx(const std::string &key, const std::string &field, const std::string &value) {
    FUTURE_CALL(TairResult<int64_t>, hsetnx, key, field, value);
}

std::future<TairResult<int64_t>> TairClientWrapper::hstrlen(const std::string &key, const std::string &field) {
    FUTURE_CALL(TairResult<int64_t>, hstrlen, key, field);
}

std::future<TairResult<ScanResult>> TairClientWrapper::hscan(const std::string &key, const std::string &cursor) {
    FUTURE_CALL(TairResult<ScanResult>, hscan, key, cursor);
}

std::future<TairResult<ScanResult>> TairClientWrapper::hscan(const std::string &key, const std::string &cursor, const ScanParams &params) {
    FUTURE_CALL(TairResult<ScanResult>, hscan, key, cursor, params);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::hrandfield(const std::string &key) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, hrandfield, key);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::hrandfield(const std::string &key, int64_t count) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, hrandfield, key, count);
}

// -------------------------------- Zset Command --------------------------------
std::future<TairResult<int64_t>> TairClientWrapper::zadd(const std::string &key, InitializerList<std::string> elements) {
    FUTURE_CALL(TairResult<int64_t>, zadd, key, elements);
}

std::future<TairResult<int64_t>> TairClientWrapper::zadd(const std::string &key, const ZAddParams &params, InitializerList<std::string> elements) {
    FUTURE_CALL(TairResult<int64_t>, zadd, key, params, elements);
}

std::future<TairResult<std::string>> TairClientWrapper::zincrby(const std::string &key, int64_t increment, const std::string &member) {
    FUTURE_CALL(TairResult<std::string>, zincrby, key, increment, member);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::zscore(const std::string &key, const std::string &member) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, zscore, key, member);
}

std::future<TairResult<std::shared_ptr<int64_t>>> TairClientWrapper::zrank(const std::string &key, const std::string &member) {
    FUTURE_CALL(TairResult<std::shared_ptr<int64_t>>, zrank, key, member);
}

std::future<TairResult<std::shared_ptr<int64_t>>> TairClientWrapper::zrevrank(const std::string &key, const std::string &member) {
    FUTURE_CALL(TairResult<std::shared_ptr<int64_t>>, zrevrank, key, member);
}

std::future<TairResult<int64_t>> TairClientWrapper::zcard(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, zcard, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::zcount(const std::string &key, const std::string &min, const std::string &max) {
    FUTURE_CALL(TairResult<int64_t>, zcount, key, min, max);
}

std::future<TairResult<int64_t>> TairClientWrapper::zlexcount(const std::string &key, const std::string &min, const std::string &max) {
    FUTURE_CALL(TairResult<int64_t>, zlexcount, key, min, max);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zpopmax(const std::string &key, int64_t count) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zpopmax, key, count);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zpopmin(const std::string &key, int64_t count) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zpopmin, key, count);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::bzpopmax(InitializerList<std::string> keys, int64_t timeout) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, bzpopmax, keys, timeout);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::bzpopmin(InitializerList<std::string> keys, int64_t timeout) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, bzpopmin, keys, timeout);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zrange(const std::string &key, const std::string &start, const std::string &stop) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zrange, key, start, stop);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zrange(const std::string &key, const std::string &start, const std::string &stop, const ZRangeParams &params) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zrange, key, start, stop, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max) {
    FUTURE_CALL(TairResult<int64_t>, zrangestore, dest, src, min, max);
}

std::future<TairResult<int64_t>> TairClientWrapper::zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ZRangeParams &params) {
    FUTURE_CALL(TairResult<int64_t>, zrangestore, dest, src, min, max, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::zrem(const std::string &key, InitializerList<std::string> members) {
    FUTURE_CALL(TairResult<int64_t>, zrem, key, members);
}

std::future<TairResult<int64_t>> TairClientWrapper::zremrange(const ZRemRangeOption &option, const std::string &key, const std::string &begin, const std::string &end) {
    FUTURE_CALL(TairResult<int64_t>, zremrange, option, key, begin, end);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zinter(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zinter, keys);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zinter(InitializerList<std::string> keys, ZInterUnionParams &params) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zinter, keys, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::zinterstore(const std::string &dest, InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, zinterstore, dest, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::zinterstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params) {
    FUTURE_CALL(TairResult<int64_t>, zinterstore, dest, keys, params);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zunion(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zunion, keys);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zunion(InitializerList<std::string> keys, ZInterUnionParams &params) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zunion, keys, params);
}

std::future<TairResult<int64_t>> TairClientWrapper::zunionstore(const std::string &dest, InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, zunionstore, dest, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::zunionstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params) {
    FUTURE_CALL(TairResult<int64_t>, zunionstore, dest, keys, params);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zdiff(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zdiff, keys);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zdiffWithScores(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zdiffWithScores, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::zdiffstore(const std::string &dest, InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, zdiffstore, dest, keys);
}

std::future<TairResult<int64_t>> TairClientWrapper::zdiffstoreWithScores(const std::string &dest, InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, zdiffstoreWithScores, dest, keys);
}

std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> TairClientWrapper::zmscore(const std::string &key, InitializerList<std::string> members) {
    FUTURE_CALL(TairResult<std::vector<std::shared_ptr<std::string>>>, zmscore, key, members);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::zrandmember(const std::string &key) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, zrandmember, key);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::zrandmember(const std::string &key, int64_t count) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, zrandmember, key, count);
}

std::future<TairResult<ScanResult>> TairClientWrapper::zscan(const std::string &key, const std::string &cursor) {
    FUTURE_CALL(TairResult<ScanResult>, zscan, key, cursor);
}

std::future<TairResult<ScanResult>> TairClientWrapper::zscan(const std::string &key, const std::string &cursor, const ScanParams &params) {
    FUTURE_CALL(TairResult<ScanResult>, zscan, key, cursor, params);
}

// -------------------------------- HyperLogLog Command --------------------------------
std::future<TairResult<int64_t>> TairClientWrapper::pfadd(const std::string &key, InitializerList<std::string> elements) {
    FUTURE_CALL(TairResult<int64_t>, pfadd, key, elements);
}

std::future<TairResult<int64_t>> TairClientWrapper::pfcount(InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<int64_t>, pfcount, keys);
}

std::future<TairResult<std::string>> TairClientWrapper::pfmerge(const std::string &dest, InitializerList<std::string> keys) {
    FUTURE_CALL(TairResult<std::string>, pfmerge, dest, keys);
}

// -------------------------------- Geo Command --------------------------------
std::future<TairResult<int64_t>> TairClientWrapper::geoadd(const std::string &key, const std::vector<std::tuple<double, double, std::string>> &members) {
    FUTURE_CALL(TairResult<int64_t>, geoadd, key, members);
}

std::future<TairResult<int64_t>> TairClientWrapper::geoadd(const std::string &key, const GeoAddParams &params, const std::vector<std::tuple<double, double, std::string>> &members) {
    FUTURE_CALL(TairResult<int64_t>, geoadd, key, params, members);
}

std::future<TairResult<std::shared_ptr<std::string>>> TairClientWrapper::geodist(const std::string &key, const std::string &member1, const std::string &member2, const GeoUnit &unit) {
    FUTURE_CALL(TairResult<std::shared_ptr<std::string>>, geodist, key, member1, member2, unit);
}

std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> TairClientWrapper::geohash(const std::string &key, InitializerList<std::string> members) {
    FUTURE_CALL(TairResult<std::vector<std::shared_ptr<std::string>>>, geohash, key, members);
}

std::future<TairResult<GeoPosResult>> TairClientWrapper::geopos(const std::string &key, InitializerList<std::string> members) {
    FUTURE_CALL(TairResult<GeoPosResult>, geopos, key, members);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::georadius(const std::string &key, double longitude, double latitude, double radius, const GeoUnit &unit) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, georadius, key, longitude, latitude, radius, unit);
}

std::future<TairResult<std::vector<std::string>>> TairClientWrapper::georadiusbymember(const std::string &key, const std::string &member, double radius, const GeoUnit &unit) {
    FUTURE_CALL(TairResult<std::vector<std::string>>, georadiusbymember, key, member, radius, unit);
}

// -------------------------------- Stream Command --------------------------------
std::future<TairResult<std::string>> TairClientWrapper::xadd(const std::string &key, const std::string &id, InitializerList<std::string> elements) {
    FUTURE_CALL(TairResult<std::string>, xadd, key, id, elements);
}

std::future<TairResult<int64_t>> TairClientWrapper::xlen(const std::string &key) {
    FUTURE_CALL(TairResult<int64_t>, xlen, key);
}

std::future<TairResult<int64_t>> TairClientWrapper::xdel(const std::string &key, InitializerList<std::string> ids) {
    FUTURE_CALL(TairResult<int64_t>, xdel, key, ids);
}

std::future<TairResult<int64_t>> TairClientWrapper::xack(const std::string &key, const std::string &group, InitializerList<std::string> ids) {
    FUTURE_CALL(TairResult<int64_t>, xack, key, group, ids);
}

std::future<TairResult<std::string>> TairClientWrapper::xgroupCreate(const std::string &key, const std::string &group, const std::string &id) {
    FUTURE_CALL(TairResult<std::string>, xgroupCreate, key, group, id);
}

std::future<TairResult<int64_t>> TairClientWrapper::xgroupCreateConsumer(const std::string &key, const std::string &group, const std::string &consumer) {
    FUTURE_CALL(TairResult<int64_t>, xgroupCreateConsumer, key, group, consumer);
}

std::future<TairResult<int64_t>> TairClientWrapper::xgroupDelConsumer(const std::string &key, const std::string &group, const std::string &consumer) {
    FUTURE_CALL(TairResult<int64_t>, xgroupDelConsumer, key, group, consumer);
}

std::future<TairResult<int64_t>> TairClientWrapper::xgroupDestroy(const std::string &key, const std::string &group) {
    FUTURE_CALL(TairResult<int64_t>, xgroupDestroy, key, group);
}

std::future<TairResult<std::string>> TairClientWrapper::xgroupSetID(const std::string &key, const std::string &group) {
    FUTURE_CALL(TairResult<std::string>, xgroupSetID, key, group);
}

std::future<TairResult<XPendingResult>> TairClientWrapper::xpending(const std::string &key, const std::string &group) {
    FUTURE_CALL(TairResult<XPendingResult>, xpending, key, group);
}

std::future<TairResult<std::vector<XRangeResult>>> TairClientWrapper::xrange(const std::string &key, const std::string &start, const std::string &end) {
    FUTURE_CALL(TairResult<std::vector<XRangeResult>>, xrange, key, start, end);
}

std::future<TairResult<std::vector<XRangeResult>>> TairClientWrapper::xrevrange(const std::string &key, const std::string &end, const std::string &start) {
    FUTURE_CALL(TairResult<std::vector<XRangeResult>>, xrevrange, key, end, start);
}

std::future<TairResult<std::vector<XReadResult>>> TairClientWrapper::xread(int64_t count, InitializerList<std::string> keys, InitializerList<std::string> ids) {
    FUTURE_CALL(TairResult<std::vector<XReadResult>>, xread, count, keys, ids);
}

std::future<TairResult<std::vector<XReadResult>>> TairClientWrapper::xreadgroup(const std::string &group, const std::string &consumer, InitializerList<std::string> keys, InitializerList<std::string> ids) {
    FUTURE_CALL(TairResult<std::vector<XReadResult>>, xreadgroup, group, consumer, keys, ids);
}

std::future<TairResult<int64_t>> TairClientWrapper::xtrim(const std::string &key, const std::string &strategy, int64_t threshold) {
    FUTURE_CALL(TairResult<int64_t>, xtrim, key, strategy, threshold);
}

std::future<TairResult<std::string>> TairClientWrapper::xsetid(const std::string &key, const std::string &last_id) {
    FUTURE_CALL(TairResult<std::string>, xsetid, key, last_id);
}

// -------------------------------- Script Command --------------------------------
std::future<PacketPtr> TairClientWrapper::eval(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args) {
    auto promise = std::make_shared<std::promise<PacketPtr>>();
    client_.eval(script, keys, args, [promise](auto *, auto &, auto &resp) { promise->set_value(resp); });
    return promise->get_future();
}

std::future<PacketPtr> TairClientWrapper::evalRo(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args) {
    auto promise = std::make_shared<std::promise<PacketPtr>>();
    client_.evalRo(script, keys, args, [promise](auto *, auto &, auto &resp) { promise->set_value(resp); });
    return promise->get_future();
}

std::future<PacketPtr> TairClientWrapper::evalsha(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args) {
    auto promise = std::make_shared<std::promise<PacketPtr>>();
    client_.evalsha(sha1, keys, args, [promise](auto *, auto &, auto &resp) { promise->set_value(resp); });
    return promise->get_future();
}

std::future<PacketPtr> TairClientWrapper::evalshaRo(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args) {
    auto promise = std::make_shared<std::promise<PacketPtr>>();
    client_.evalshaRo(sha1, keys, args, [promise](auto *, auto &, auto &resp) { promise->set_value(resp); });
    return promise->get_future();
}

std::future<TairResult<std::string>> TairClientWrapper::scriptLoad(const std::string &script) {
    FUTURE_CALL(TairResult<std::string>, scriptLoad, script);
}

std::future<TairResult<std::string>> TairClientWrapper::scriptFlush() {
    FUTURE_CALL(TairResult<std::string>, scriptFlush);
}

std::future<TairResult<std::string>> TairClientWrapper::scriptKill() {
    FUTURE_CALL(TairResult<std::string>, scriptKill);
}

std::future<TairResult<std::vector<int64_t>>> TairClientWrapper::scriptExists(InitializerList<std::string> sha1s) {
    FUTURE_CALL(TairResult<std::vector<int64_t>>, scriptExists, sha1s);
}

// -------------------------------- Connection Command --------------------------------
std::future<TairResult<std::string>> TairClientWrapper::auth(const std::string &password) {
    FUTURE_CALL(TairResult<std::string>, auth, password);
}

std::future<TairResult<std::string>> TairClientWrapper::auth(const std::string &user, const std::string &password) {
    FUTURE_CALL(TairResult<std::string>, auth, user, password);
}

std::future<TairResult<std::string>> TairClientWrapper::quit() {
    FUTURE_CALL(TairResult<std::string>, quit);
}

// -------------------------------- Server Command --------------------------------
std::future<TairResult<std::string>> TairClientWrapper::flushall() {
    FUTURE_CALL(TairResult<std::string>, flushall);
}

// -------------------------------- Pubsub Command --------------------------------
std::future<TairResult<int64_t>> TairClientWrapper::publish(const std::string &channel, const std::string &message) {
    FUTURE_CALL(TairResult<int64_t>, publish, channel, message);
}

std::future<TairResult<int64_t>> TairClientWrapper::clusterPublish(const std::string &channel, const std::string &message, const std::string &name, int flag) {
    FUTURE_CALL(TairResult<int64_t>, clusterPublish, channel, message, name, flag);
}

// -------------------------------- Cluster Command --------------------------------
std::future<TairResult<std::string>> TairClientWrapper::clusterNodes() {
    FUTURE_CALL(TairResult<std::string>, clusterNodes);
}

} // namespace tair::client
