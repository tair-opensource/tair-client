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

#include <array>
#include <initializer_list>
#include <unordered_map>

#include "common/KeyHash.hpp"
#include "common/Noncopyable.hpp"
#include "client/TairAsyncClient.hpp"
#include "client/interface/ITairClient.hpp"

namespace tair::client {

using common::KeyHash;
using common::Noncopyable;

using TairAsyncClientPtr = std::shared_ptr<TairAsyncClient>;
using TairClientMap = std::unordered_map<std::string, TairAsyncClientPtr>;

class TairClusterAsyncClient : public ITairClient, private Noncopyable {
public:
    TairClusterAsyncClient();
    explicit TairClusterAsyncClient(EventLoop *loop);
    ~TairClusterAsyncClient() override;

    TairResult<std::string> init() override;
    void destroy() override;

    void setServerAddr(const std::string &addr) override;
    const std::string &getServerAddr() const override;

    void setUser(const std::string &user) override;
    void setPassword(const std::string &password) override;
    void setConnectingTimeoutMs(int timeout_ms) override;
    void setReconnectIntervalMs(int timeout_ms) override;
    void setAutoReconnect(bool reconnect) override;
    void setKeepAliveSeconds(int seconds) override;

    // send command
    void sendCommand(CommandArgv &&argv, const ResultPacketCallback &callback) override;
    void sendCommand(const CommandArgv &argv, const ResultPacketCallback &callback) override;

    void sendCommand(CommandArgv &&argv, const ResultPacketAndLatencyCallback &callback) override;
    void sendCommand(const CommandArgv &argv, const ResultPacketAndLatencyCallback &callback) override;

    // generic
    void del(const std::string &key, const ResultIntegerCallback &callback) override;
    void del(InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void unlink(const std::string &key, const ResultIntegerCallback &callback) override;
    void unlink(InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void exists(const std::string &key, const ResultIntegerCallback &callback) override;
    void exists(InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void expire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback) override;
    void expire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback) override;
    void expireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback) override;
    void expireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback) override;
    void persist(const std::string &key, const ResultIntegerCallback &callback) override;
    void pexpire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback) override;
    void pexpire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback) override;
    void pexpireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback) override;
    void pexpireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback) override;
    void ttl(const std::string &key, const ResultIntegerCallback &callback) override;
    void pttl(const std::string &key, const ResultIntegerCallback &callback) override;
    void touch(InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void dump(const std::string &key, const ResultStringPtrCallback &callback) override;
    void restore(const std::string &key, int64_t ttl, const std::string &value, const ResultStringCallback &callback) override;
    void restore(const std::string &key, int64_t ttl, const std::string &value, const RestoreParams &params, const ResultStringCallback &callback) override;
    void keys(const std::string &pattern, const ResultVectorStringCallback &callback) override;
    void move(const std::string &key, int64_t db, const ResultIntegerCallback &callback) override;
    void randomkey(const ResultStringPtrCallback &callback) override;
    void rename(const std::string &key, const std::string &newkey, const ResultStringCallback &callback) override;
    void renamenx(const std::string &key, const std::string &newkey, const ResultIntegerCallback &callback) override;
    void type(const std::string &key, const ResultStringCallback &callback) override;
    void scan(const std::string &cursor, const ResultScanCallback &callback) override;
    void scan(const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) override;
    void sort(const std::string &key, const ResultVectorStringPtrCallback &callback) override;
    void sort(const std::string &key, const SortParams &params, const ResultVectorStringPtrCallback &callback) override;
    void sortStore(const std::string &key, const std::string &storekey, const ResultIntegerCallback &callback) override;
    void sortStore(const std::string &key, const std::string &storekey, const SortParams &params, const ResultIntegerCallback &callback) override;
    void copy(const std::string &key, const std::string &destkey, const ResultIntegerCallback &callback) override;
    void copy(const std::string &key, const std::string &destkey, const CopyParams &params, const ResultIntegerCallback &callback) override;

    // string
    void append(const std::string &key, const std::string &value, const ResultIntegerCallback &callback) override;
    void bitcount(const std::string &key, const ResultIntegerCallback &callback) override;
    void bitcount(const std::string &key, const BitPositonParams &params, const ResultIntegerCallback &callback) override;
    void bitfield(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback) override;
    void bitfieldRo(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback) override;
    void bitop(const BitOperation &op, InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void bitpos(const std::string &key, int64_t bit, const ResultIntegerCallback &callback) override;
    void bitpos(const std::string &key, int64_t bit, const BitPositonParams &params, const ResultIntegerCallback &callback) override;
    void setbit(const std::string &key, int64_t offset, int64_t value, const ResultIntegerCallback &callback) override;
    void getbit(const std::string &key, int64_t offset, const ResultIntegerCallback &callback) override;
    void decr(const std::string &key, const ResultIntegerCallback &callback) override;
    void decrby(const std::string &key, int64_t decrement, const ResultIntegerCallback &callback) override;
    void getrange(const std::string &key, int64_t start, int64_t end, const ResultStringCallback &callback) override;
    void getset(const std::string &key, const std::string &value, const ResultStringPtrCallback &callback) override;
    void set(const std::string &key, const std::string &value, const SetParams &params, const ResultStringCallback &callback) override;
    void set(const std::string &key, const std::string &value, const ResultStringCallback &callback) override;
    void get(const std::string &key, const ResultStringPtrCallback &callback) override;
    void incr(const std::string &key, const ResultIntegerCallback &callback) override;
    void incrby(const std::string &key, int64_t increment, const ResultIntegerCallback &callback) override;
    void incrbyfloat(const std::string &key, double increment, const ResultStringCallback &callback) override;
    void mget(InitializerList<std::string> keys, const ResultVectorStringPtrCallback &callback) override;
    void mset(InitializerList<std::string> kvs, const ResultStringCallback &callback) override;
    void msetnx(InitializerList<std::string> kvs, const ResultIntegerCallback &callback) override;
    void psetex(const std::string &key, int64_t milliseconds, const std::string &value, const ResultStringCallback &callback) override;
    void setex(const std::string &key, int64_t seconds, const std::string &value, const ResultStringCallback &callback) override;
    void setnx(const std::string &key, const std::string &value, const ResultIntegerCallback &callback) override;
    void setrange(const std::string &key, int64_t offset, const std::string &value, const ResultIntegerCallback &callback) override;
    void strlen(const std::string &key, const ResultIntegerCallback &callback) override;
    void getdel(const std::string &key, const ResultStringPtrCallback &callback) override;
    void getex(const std::string &key, const GetExParams &params, const ResultStringPtrCallback &callback) override;

    // list
    void blpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) override;
    void brpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) override;
    void brpoplpush(const std::string &src, const std::string &dest, int64_t timeout, const ResultStringPtrCallback &callback) override;
    void lindex(const std::string &key, int64_t index, const ResultStringPtrCallback &callback) override;
    void linsert(const std::string &key, const ListDirection &direction, const std::string &pivot, const std::string &element, const ResultIntegerCallback &callback) override;
    void llen(const std::string &key, const ResultIntegerCallback &callback) override;
    void lpop(const std::string &key, const ResultStringPtrCallback &callback) override;
    void lpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) override;
    void lpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) override;
    void lpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) override;
    void lrange(const std::string &key, int64_t start, int64_t stop, const ResultVectorStringCallback &callback) override;
    void lrem(const std::string &key, int64_t count, const std::string &element, const ResultIntegerCallback &callback) override;
    void lset(const std::string &key, int64_t index, const std::string &element, const ResultStringCallback &callback) override;
    void ltrim(const std::string &key, int64_t start, int64_t stop, const ResultStringCallback &callback) override;
    void rpop(const std::string &key, const ResultStringPtrCallback &callback) override;
    void rpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) override;
    void rpoplpush(const std::string &src, const std::string &dest, const ResultStringPtrCallback &callback) override;
    void rpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) override;
    void rpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) override;
    void lmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, const ResultStringPtrCallback &callback) override;
    void blmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, int64_t timeout, const ResultStringPtrCallback &callback) override;

    // set
    void sadd(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) override;
    void scard(const std::string &key, const ResultIntegerCallback &callback) override;
    void sismember(const std::string &key, const std::string &member, const ResultIntegerCallback &callback) override;
    void smembers(const std::string &key, const ResultVectorStringCallback &callback) override;
    void smove(const std::string &src, const std::string &dest, const std::string &member, const ResultIntegerCallback &callback) override;
    void srem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) override;
    void spop(const std::string &key, const ResultStringPtrCallback &callback) override;
    void spop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) override;
    void srandmember(const std::string &key, const ResultStringPtrCallback &callback) override;
    void srandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) override;
    void sdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) override;
    void sinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) override;
    void sunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) override;
    void sdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void sinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void sunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void sscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) override;
    void sscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) override;
    void smismember(const std::string &key, InitializerList<std::string> members, const ResultVectorIntegerCallback &callback) override;

    // hash
    void hdel(const std::string &key, InitializerList<std::string> fields, const ResultIntegerCallback &callback) override;
    void hexists(const std::string &key, const std::string &field, const ResultIntegerCallback &callback) override;
    void hget(const std::string &key, const std::string &field, const ResultStringPtrCallback &callback) override;
    void hgetall(const std::string &key, const ResultVectorStringCallback &callback) override;
    void hincrby(const std::string &key, const std::string &field, int64_t increment, const ResultIntegerCallback &callback) override;
    void hincrbyfloat(const std::string &key, const std::string &field, double increment, const ResultStringCallback &callback) override;
    void hkeys(const std::string &key, const ResultVectorStringCallback &callback) override;
    void hvals(const std::string &key, const ResultVectorStringCallback &callback) override;
    void hlen(const std::string &key, const ResultIntegerCallback &callback) override;
    void hmget(const std::string &key, InitializerList<std::string> fields, const ResultVectorStringPtrCallback &callback) override;
    void hset(const std::string &key, const std::string &filed, const std::string &value, const ResultIntegerCallback &callback) override;
    void hset(const std::string &key, InitializerList<std::string> kvs, const ResultIntegerCallback &callback) override;
    void hsetnx(const std::string &key, const std::string &field, const std::string &value, const ResultIntegerCallback &callback) override;
    void hstrlen(const std::string &key, const std::string &field, const ResultIntegerCallback &callback) override;
    void hscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) override;
    void hscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) override;
    void hrandfield(const std::string &key, const ResultStringPtrCallback &callback) override;
    void hrandfield(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) override;

    // zset
    void zadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) override;
    void zadd(const std::string &key, const ZAddParams &params, InitializerList<std::string> elements, const ResultIntegerCallback &callback) override;
    void zincrby(const std::string &key, int64_t increment, const std::string &member, const ResultStringCallback &callback) override;
    void zscore(const std::string &key, const std::string &member, const ResultStringPtrCallback &callback) override;
    void zrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback) override;
    void zrevrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback) override;
    void zcard(const std::string &key, const ResultIntegerCallback &callback) override;
    void zcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) override;
    void zlexcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) override;
    void zpopmax(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) override;
    void zpopmin(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) override;
    void bzpopmax(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) override;
    void bzpopmin(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) override;
    void zrange(const std::string &key, const std::string &start, const std::string &stop, const ResultVectorStringCallback &callback) override;
    void zrange(const std::string &key, const std::string &start, const std::string &stop, const ZRangeParams &params, const ResultVectorStringCallback &callback) override;
    void zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) override;
    void zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ZRangeParams &params, const ResultIntegerCallback &callback) override;
    void zrem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) override;
    void zremrange(const ZRemRangeOption &option, const std::string &key, const std::string &begin, const std::string &end, const ResultIntegerCallback &callback) override;
    void zinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) override;
    void zinter(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback) override;
    void zinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void zinterstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback) override;
    void zunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) override;
    void zunion(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback) override;
    void zunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void zunionstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback) override;
    void zdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) override;
    void zdiffWithScores(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) override;
    void zdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void zdiffstoreWithScores(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void zmscore(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback) override;
    void zrandmember(const std::string &key, const ResultStringPtrCallback &callback) override;
    void zrandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) override;
    void zscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) override;
    void zscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) override;

    // hyperloglog
    void pfadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) override;
    void pfcount(InitializerList<std::string> keys, const ResultIntegerCallback &callback) override;
    void pfmerge(const std::string &dest, InitializerList<std::string> keys, const ResultStringCallback &callback) override;

    // geo
    void geoadd(const std::string &key, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback) override;
    void geoadd(const std::string &key, const GeoAddParams &params, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback) override;
    void geodist(const std::string &key, const std::string &member1, const std::string &member2, const GeoUnit &unit, const ResultStringPtrCallback &callback) override;
    void geohash(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback) override;
    void geopos(const std::string &key, InitializerList<std::string> members, const ResultGeoposCallback &callback) override;
    void georadius(const std::string &key, double longitude, double latitude, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback) override;
    void georadiusbymember(const std::string &key, const std::string &member, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback) override;

    // stream
    void xadd(const std::string &key, const std::string &id, InitializerList<std::string> elements, const ResultStringCallback &callback) override;
    void xlen(const std::string &key, const ResultIntegerCallback &callback) override;
    void xdel(const std::string &key, InitializerList<std::string> ids, const ResultIntegerCallback &callback) override;
    void xack(const std::string &key, const std::string &group, InitializerList<std::string> ids, const ResultIntegerCallback &callback) override;
    void xgroupCreate(const std::string &key, const std::string &group, const std::string &id, const ResultStringCallback &callback) override;
    void xgroupCreateConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback) override;
    void xgroupDelConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback) override;
    void xgroupDestroy(const std::string &key, const std::string &group, const ResultIntegerCallback &callback) override;
    void xgroupSetID(const std::string &key, const std::string &group, const ResultStringCallback &callback) override;
    void xpending(const std::string &key, const std::string &group, const ResultXPendingCallback &callback) override;
    void xrange(const std::string &key, const std::string &start, const std::string &end, const ResultXRangeCallback &callback) override;
    void xrevrange(const std::string &key, const std::string &end, const std::string &start, const ResultXRangeCallback &callback) override;
    void xread(int64_t count, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback) override;
    void xreadgroup(const std::string &group, const std::string &consumer, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback) override;
    void xtrim(const std::string &key, const std::string &strategy, int64_t threshold, const ResultIntegerCallback &callback) override;
    void xsetid(const std::string &key, const std::string &last_id, const ResultStringCallback &callback) override;

    // script
    void eval(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) override;
    void evalRo(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) override;
    void evalsha(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) override;
    void evalshaRo(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) override;
    void scriptLoad(const std::string &script, const ResultStringCallback &callback) override;
    void scriptFlush(const ResultStringCallback &callback) override;
    void scriptKill(const ResultStringCallback &callback) override;
    void scriptExists(InitializerList<std::string> sha1s, const ResultVectorIntegerCallback &callback) override;

    // connection
    void auth(const std::string &password, const ResultStringCallback &callback) override;
    void auth(const std::string &user, const std::string &password, const ResultStringCallback &callback) override;
    void quit(const ResultStringCallback &callback) override;

    // server
    void flushall(const ResultStringCallback &callback) override;

    // pubsub
    void publish(const std::string &channel, const std::string &message, const ResultIntegerCallback &callback) override;
    void clusterPublish(const std::string &channel, const std::string &message,
                        const std::string &name, int flag, const ResultIntegerCallback &callback) override;

    // cluster
    void clusterNodes(const ResultStringCallback &callback) override;

private:
    void initEventLoop();
    bool checkResultHasClusterError(const PacketPtr &resp);
    static int calcCommandSlot(const CommandArgv &argv);
    TairAsyncClientPtr getClientByKey(const std::string &key);
    TairAsyncClientPtr getClientRandom();
    bool checkSlotToClients();
    bool parseNodesInfoAndInitClient(std::string &nodes_info);
    bool getClusterNodesInfo(const TairAsyncClientPtr &client, std::string &nodes_info);
    TairAsyncClientPtr createClient(const std::string &addr);
    bool checkKeyInSameSlot(std::initializer_list<std::string> list);
    bool checkKeyInSameSlot(const std::string &dest, std::initializer_list<std::string> list);

private:
    // User parameter
    std::string server_addr_;
    std::string user_;
    std::string password_;
    int connecting_timeout_ms_ = 2000;
    int reconnect_interval_ms_ = -1;
    bool auto_reconnect_ = true;
    int keepalive_seconds_ = 60;

    // Cluster resource
    std::unique_ptr<EventLoopThread> loop_thread_;
    EventLoop *loop_ = nullptr;
    TairClientMap client_map_;
    std::array<TairAsyncClientPtr, KeyHash::SLOTS_NUM> slot_to_clients_ = {nullptr};
};

} // namespace tair::client
