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

#include "client/TairClientDefine.hpp"
#include "client/TairClientWrapper.hpp"
#include "client/TairURI.hpp"
#include "client/params/ParamsAll.hpp"

namespace tair::client {

class ITairClient;

/// @brief TairClient can connect to Standalone/Cluster/Sentinel Tair.
class TairClient {
public:
    explicit TairClient() = default;
    ~TairClient();

    /// @brief `TairClient` is not copyable.
    TairClient(const TairClient &) = delete;
    TairClient &operator=(const TairClient &) = delete;

    /// @brief `TairClient` is movable.
    TairClient(TairClient &&) = default;
    TairClient &operator=(TairClient &&) = default;

    /// @brief Initialize connection to Tair.
    /// @param uri Parameters for connecting Tair, e.g. to build uri
    /// TairURI::create().type(TairURI::STANDALONE).host("localhost").port(6379).build();
    /// @return Whether init is successful.
    /// @retval true If connection succeeded.
    /// @retval false The connection failed, please check the tair.log for the reason of the error.
    /// @see `TairURI`
    TairResult<std::string> init(const TairURI &uri);

    /// @brief Disconnect Tair and destruct resources, usually called when the application exits.
    void destroy();

    /// @brief Return TairClientWrapper for Future Mode API
    TairClientWrapper getFutureWrapper();

    // -------------------------------- send Command --------------------------------
    void sendCommand(CommandArgv &&argv, const ResultPacketCallback &callback);
    void sendCommand(const CommandArgv &argv, const ResultPacketCallback &callback);

    void sendCommand(CommandArgv &&argv, const ResultPacketAndLatencyCallback &callback);
    void sendCommand(const CommandArgv &argv, const ResultPacketAndLatencyCallback &callback);

    // -------------------------------- Generic Command --------------------------------
    /// @brief Delete a key.
    /// @param key The key.
    /// @param callback The integer callback.
    /// @param callback 1 If the key exists, and has been deleted.
    /// @param callback 0 If the key does not exist.
    void del(const std::string &key, const ResultIntegerCallback &callback);

    /// @brief Delete some keys.
    /// @param keys Initializer list of keys to be deleted.
    /// @param callback The integer callback.
    /// @param callback Number of keys deleted.
    void del(InitializerList<std::string> keys, const ResultIntegerCallback &callback);

    /// @brief Delete the given key asynchronously.
    /// @param key The key.
    /// @param callback The integer callback.
    /// @param callback 1 If key exists, and has been deleted.
    /// @param callback 0 If key does not exist.
    void unlink(const std::string &key, const ResultIntegerCallback &callback);

    /// @brief Delete some keys asynchronously.
    /// @param keys Initializer list of keys to be deleted.
    /// @param callback The integer callback
    /// @param callback Number of keys deleted.
    void unlink(InitializerList<std::string> keys, const ResultIntegerCallback &callback);

    /// @brief Check if the given key exists.
    /// @param key Key.
    /// @param callback The integer callback.
    /// @param callback 1 If key exists.
    /// @param callback 0 If key does not exist.
    void exists(const std::string &key, const ResultIntegerCallback &callback);

    /// @brief Check if the given keys exists.
    /// @param keys Initializer list of keys to be checked.
    /// @param callback The integer callback.
    /// @param callback  Number of keys existing.
    void exists(InitializerList<std::string> keys, const ResultIntegerCallback &callback);

    /// @brief Set a timeout on key.
    /// @param key Key.
    /// @param timeout Timeout in seconds.
    /// @param callback The integer callback.
    /// @param callback 1 If timeout has been set.
    /// @param callback 0 If key does not exist.
    void expire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback);

    /// @brief Set a timeout on key.
    /// @param key Key.
    /// @param timeout Timeout in seconds.
    /// @param params @see `ExpireParams`
    /// @param callback The integer callback.
    /// @param callback 1 If timeout has been set.
    /// @param callback 0 If key does not exist.
    void expire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback);

    void expireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback);
    void expireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback);
    void persist(const std::string &key, const ResultIntegerCallback &callback);
    void pexpire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback);
    void pexpire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback);
    void pexpireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback);
    void pexpireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback);
    void ttl(const std::string &key, const ResultIntegerCallback &callback);
    void pttl(const std::string &key, const ResultIntegerCallback &callback);
    void touch(InitializerList<std::string> keys, const ResultIntegerCallback &callback);
    void dump(const std::string &key, const ResultStringPtrCallback &callback);
    void restore(const std::string &key, int64_t ttl, const std::string &value, const ResultStringCallback &callback);
    void restore(const std::string &key, int64_t ttl, const std::string &value, const RestoreParams &params, const ResultStringCallback &callback);
    void keys(const std::string &pattern, const ResultVectorStringCallback &callback);
    void move(const std::string &key, int64_t db, const ResultIntegerCallback &callback);
    void randomkey(const ResultStringPtrCallback &callback);
    void rename(const std::string &key, const std::string &newkey, const ResultStringCallback &callback);
    void renamenx(const std::string &key, const std::string &newkey, const ResultIntegerCallback &callback);
    void type(const std::string &key, const ResultStringCallback &callback);
    void scan(const std::string &cursor, const ResultScanCallback &callback);
    void scan(const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback);
    void sort(const std::string &key, const ResultVectorStringPtrCallback &callback);
    void sort(const std::string &key, const SortParams &params, const ResultVectorStringPtrCallback &callback);
    void sortStore(const std::string &key, const std::string &storekey, const ResultIntegerCallback &callback);
    void sortStore(const std::string &key, const std::string &storekey, const SortParams &params, const ResultIntegerCallback &callback);
    void copy(const std::string &key, const std::string &destkey, const ResultIntegerCallback &callback);
    void copy(const std::string &key, const std::string &destkey, const CopyParams &params, const ResultIntegerCallback &callback);

    // -------------------------------- String Command --------------------------------
    void append(const std::string &key, const std::string &value, const ResultIntegerCallback &callback);
    void bitcount(const std::string &key, const ResultIntegerCallback &callback);
    void bitcount(const std::string &key, const BitPositonParams &params, const ResultIntegerCallback &callback);
    void bitfield(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback);
    void bitfieldRo(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback);
    void bitop(const BitOperation &op, InitializerList<std::string> keys, const ResultIntegerCallback &callback);
    void bitpos(const std::string &key, int64_t bit, const ResultIntegerCallback &callback);
    void bitpos(const std::string &key, int64_t bit, const BitPositonParams &params, const ResultIntegerCallback &callback);
    void setbit(const std::string &key, int64_t offset, int64_t value, const ResultIntegerCallback &callback);
    void getbit(const std::string &key, int64_t offset, const ResultIntegerCallback &callback);
    void decr(const std::string &key, const ResultIntegerCallback &callback);
    void decrby(const std::string &key, int64_t decrement, const ResultIntegerCallback &callback);
    void getrange(const std::string &key, int64_t start, int64_t end, const ResultStringCallback &callback);
    void getset(const std::string &key, const std::string &value, const ResultStringPtrCallback &callback);
    void set(const std::string &key, const std::string &value, const SetParams &params, const ResultStringCallback &callback);
    /// @brief Set a key-value pair.
    /// @param key The key.
    /// @param value The value.
    /// @param callback The string callback.
    /// @param OK If the key has been set.
    /// @param other errors means fail.
    void set(const std::string &key, const std::string &value, const ResultStringCallback &callback);

    /// @brief Get value by key.
    /// @param key The key.
    /// @param callback The string callback.
    /// @param callback The result value.
    void get(const std::string &key, const ResultStringPtrCallback &callback);
    void incr(const std::string &key, const ResultIntegerCallback &callback);
    void incrby(const std::string &key, int64_t increment, const ResultIntegerCallback &callback);
    void incrbyfloat(const std::string &key, double increment, const ResultStringCallback &callback);
    void mget(InitializerList<std::string> keys, const ResultVectorStringPtrCallback &callback);
    void mset(InitializerList<std::string> kvs, const ResultStringCallback &callback);
    void msetnx(InitializerList<std::string> kvs, const ResultIntegerCallback &callback);
    void psetex(const std::string &key, int64_t milliseconds, const std::string &value, const ResultStringCallback &callback);
    void setex(const std::string &key, int64_t seconds, const std::string &value, const ResultStringCallback &callback);
    void setnx(const std::string &key, const std::string &value, const ResultIntegerCallback &callback);
    void setrange(const std::string &key, int64_t offset, const std::string &value, const ResultIntegerCallback &callback);
    void strlen(const std::string &key, const ResultIntegerCallback &callback);
    void getdel(const std::string &key, const ResultStringPtrCallback &callback);
    void getex(const std::string &key, const GetExParams &params, const ResultStringPtrCallback &callback);

    // -------------------------------- List Command --------------------------------
    void blpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback);
    void brpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback);
    void brpoplpush(const std::string &src, const std::string &dest, int64_t timeout, const ResultStringPtrCallback &callback);
    void lindex(const std::string &key, int64_t index, const ResultStringPtrCallback &callback);
    void linsert(const std::string &key, const ListDirection &direction, const std::string &pivot, const std::string &element, const ResultIntegerCallback &callback);
    void llen(const std::string &key, const ResultIntegerCallback &callback);
    void lpop(const std::string &key, const ResultStringPtrCallback &callback);
    void lpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback);
    void lpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback);
    void lpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback);
    void lrange(const std::string &key, int64_t start, int64_t stop, const ResultVectorStringCallback &callback);
    void lrem(const std::string &key, int64_t count, const std::string &element, const ResultIntegerCallback &callback);
    void lset(const std::string &key, int64_t index, const std::string &element, const ResultStringCallback &callback);
    void ltrim(const std::string &key, int64_t start, int64_t stop, const ResultStringCallback &callback);
    void rpop(const std::string &key, const ResultStringPtrCallback &callback);
    void rpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback);
    void rpoplpush(const std::string &src, const std::string &dest, const ResultStringPtrCallback &callback);
    void rpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback);
    void rpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback);
    void lmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, const ResultStringPtrCallback &callback);
    void blmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, int64_t timeout, const ResultStringPtrCallback &callback);

    // -------------------------------- Set Command --------------------------------
    void sadd(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback);
    void scard(const std::string &key, const ResultIntegerCallback &callback);
    void sismember(const std::string &key, const std::string &member, const ResultIntegerCallback &callback);
    void smembers(const std::string &key, const ResultVectorStringCallback &callback);
    void smove(const std::string &src, const std::string &dest, const std::string &member, const ResultIntegerCallback &callback);
    void srem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback);
    void spop(const std::string &key, const ResultStringPtrCallback &callback);
    void spop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback);
    void srandmember(const std::string &key, const ResultStringPtrCallback &callback);
    void srandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback);
    void sdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback);
    void sinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback);
    void sunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback);
    void sdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback);
    void sinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback);
    void sunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback);
    void sscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback);
    void sscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback);
    void smismember(const std::string &key, InitializerList<std::string> members, const ResultVectorIntegerCallback &callback);

    // -------------------------------- Hash Command --------------------------------
    void hdel(const std::string &key, InitializerList<std::string> fields, const ResultIntegerCallback &callback);
    void hexists(const std::string &key, const std::string &field, const ResultIntegerCallback &callback);
    void hget(const std::string &key, const std::string &field, const ResultStringPtrCallback &callback);
    void hgetall(const std::string &key, const ResultVectorStringCallback &callback);
    void hincrby(const std::string &key, const std::string &field, int64_t increment, const ResultIntegerCallback &callback);
    void hincrbyfloat(const std::string &key, const std::string &field, double increment, const ResultStringCallback &callback);
    void hkeys(const std::string &key, const ResultVectorStringCallback &callback);
    void hvals(const std::string &key, const ResultVectorStringCallback &callback);
    void hlen(const std::string &key, const ResultIntegerCallback &callback);
    void hmget(const std::string &key, InitializerList<std::string> fields, const ResultVectorStringPtrCallback &callback);
    void hset(const std::string &key, const std::string &filed, const std::string &value, const ResultIntegerCallback &callback);
    void hset(const std::string &key, InitializerList<std::string> kvs, const ResultIntegerCallback &callback);
    void hsetnx(const std::string &key, const std::string &field, const std::string &value, const ResultIntegerCallback &callback);
    void hstrlen(const std::string &key, const std::string &field, const ResultIntegerCallback &callback);
    void hscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback);
    void hscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback);
    void hrandfield(const std::string &key, const ResultStringPtrCallback &callback);
    void hrandfield(const std::string &key, int64_t count, const ResultVectorStringCallback &callback);

    // -------------------------------- Zset Command --------------------------------
    void zadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback);
    void zadd(const std::string &key, const ZAddParams &params, InitializerList<std::string> elements, const ResultIntegerCallback &callback);
    void zincrby(const std::string &key, int64_t increment, const std::string &member, const ResultStringCallback &callback);
    void zscore(const std::string &key, const std::string &member, const ResultStringPtrCallback &callback);
    void zrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback);
    void zrevrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback);
    void zcard(const std::string &key, const ResultIntegerCallback &callback);
    void zcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback);
    void zlexcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback);
    void zpopmax(const std::string &key, int64_t count, const ResultVectorStringCallback &callback);
    void zpopmin(const std::string &key, int64_t count, const ResultVectorStringCallback &callback);
    void bzpopmax(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback);
    void bzpopmin(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback);
    void zrange(const std::string &key, const std::string &start, const std::string &stop, const ResultVectorStringCallback &callback);
    void zrange(const std::string &key, const std::string &start, const std::string &stop, const ZRangeParams &params, const ResultVectorStringCallback &callback);
    void zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ResultIntegerCallback &callback);
    void zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ZRangeParams &params, const ResultIntegerCallback &callback);
    void zrem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback);
    void zremrange(const ZRemRangeOption &option, const std::string &key, const std::string &begin, const std::string &end, const ResultIntegerCallback &callback);
    void zinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback);
    void zinter(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback);
    void zinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback);
    void zinterstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback);
    void zunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback);
    void zunion(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback);
    void zunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback);
    void zunionstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback);
    void zdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback);
    void zdiffWithScores(InitializerList<std::string> keys, const ResultVectorStringCallback &callback);
    void zdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback);
    void zdiffstoreWithScores(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback);
    void zmscore(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback);
    void zrandmember(const std::string &key, const ResultStringPtrCallback &callback);
    void zrandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback);
    void zscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback);
    void zscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback);

    // -------------------------------- HyperLogLog Command --------------------------------
    void pfadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback);
    void pfcount(InitializerList<std::string> keys, const ResultIntegerCallback &callback);
    void pfmerge(const std::string &dest, InitializerList<std::string> keys, const ResultStringCallback &callback);

    // -------------------------------- Geo Command --------------------------------
    void geoadd(const std::string &key, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback);
    void geoadd(const std::string &key, const GeoAddParams &params, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback);
    void geodist(const std::string &key, const std::string &member1, const std::string &member2, const GeoUnit &unit, const ResultStringPtrCallback &callback);
    void geohash(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback);
    void geopos(const std::string &key, InitializerList<std::string> members, const ResultGeoposCallback &callback);
    void georadius(const std::string &key, double longitude, double latitude, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback);
    void georadiusbymember(const std::string &key, const std::string &member, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback);

    // -------------------------------- Stream Command --------------------------------
    void xadd(const std::string &key, const std::string &id, InitializerList<std::string> elements, const ResultStringCallback &callback);
    void xlen(const std::string &key, const ResultIntegerCallback &callback);
    void xdel(const std::string &key, InitializerList<std::string> ids, const ResultIntegerCallback &callback);
    void xack(const std::string &key, const std::string &group, InitializerList<std::string> ids, const ResultIntegerCallback &callback);
    void xgroupCreate(const std::string &key, const std::string &group, const std::string &id, const ResultStringCallback &callback);
    void xgroupCreateConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback);
    void xgroupDelConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback);
    void xgroupDestroy(const std::string &key, const std::string &group, const ResultIntegerCallback &callback);
    void xgroupSetID(const std::string &key, const std::string &group, const ResultStringCallback &callback);
    void xpending(const std::string &key, const std::string &group, const ResultXPendingCallback &callback);
    void xrange(const std::string &key, const std::string &start, const std::string &end, const ResultXRangeCallback &callback);
    void xrevrange(const std::string &key, const std::string &end, const std::string &start, const ResultXRangeCallback &callback);
    void xread(int64_t count, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback);
    void xreadgroup(const std::string &group, const std::string &consumer, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback);
    void xtrim(const std::string &key, const std::string &strategy, int64_t threshold, const ResultIntegerCallback &callback);
    void xsetid(const std::string &key, const std::string &last_id, const ResultStringCallback &callback);

    // -------------------------------- Script Command --------------------------------
    void eval(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback);
    void evalRo(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback);
    void evalsha(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback);
    void evalshaRo(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback);
    void scriptLoad(const std::string &script, const ResultStringCallback &callback);
    void scriptFlush(const ResultStringCallback &callback);
    void scriptKill(const ResultStringCallback &callback);
    void scriptExists(InitializerList<std::string> sha1s, const ResultVectorIntegerCallback &callback);

    // -------------------------------- Connection Command --------------------------------
    void auth(const std::string &password, const ResultStringCallback &callback);
    void auth(const std::string &user, const std::string &password, const ResultStringCallback &callback);
    void quit(const ResultStringCallback &callback);

    // -------------------------------- Server Command --------------------------------
    void flushall(const ResultStringCallback &callback);

    // -------------------------------- Pubsub Command --------------------------------
    void publish(const std::string &channel, const std::string &message, const ResultIntegerCallback &callback);
    void clusterPublish(const std::string &channel, const std::string &message, const std::string &name, int flag, const ResultIntegerCallback &callback);

    // -------------------------------- Cluster Command --------------------------------
    void clusterNodes(const ResultStringCallback &callback);

private:
    ITairClient *itair_ = nullptr;
};

} // namespace tair::client