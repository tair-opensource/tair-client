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
#include "client/TairResult.hpp"
#include "client/params/ParamsAll.hpp"
#include "client/results/ResultsAll.hpp"

namespace tair::client {

class ITairClient {
public:
    ITairClient() = default;
    virtual ~ITairClient() = default;

    virtual TairResult<std::string> init() = 0;
    virtual void destroy() = 0;

    virtual void setServerAddr(const std::string &addr) = 0;
    virtual const std::string &getServerAddr() const = 0;

    virtual void setUser(const std::string &user) = 0;
    virtual void setPassword(const std::string &password) = 0;
    virtual void setConnectingTimeoutMs(int timeout_ms) = 0;
    virtual void setReconnectIntervalMs(int timeout_ms) = 0;
    virtual void setAutoReconnect(bool reconnect) = 0;
    virtual void setKeepAliveSeconds(int seconds) = 0;

    // send command
    virtual void sendCommand(CommandArgv &&argv, const ResultPacketCallback &callback) = 0;
    virtual void sendCommand(const CommandArgv &argv, const ResultPacketCallback &callback) = 0;

    virtual void sendCommand(CommandArgv &&argv, const ResultPacketAndLatencyCallback &callback) = 0;
    virtual void sendCommand(const CommandArgv &argv, const ResultPacketAndLatencyCallback &callback) = 0;

    // generic
    virtual void del(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void del(InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void unlink(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void unlink(InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void exists(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void exists(InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void expire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback) = 0;
    virtual void expire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback) = 0;
    virtual void expireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback) = 0;
    virtual void expireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback) = 0;
    virtual void persist(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void pexpire(const std::string &key, int64_t timeout, const ResultIntegerCallback &callback) = 0;
    virtual void pexpire(const std::string &key, int64_t timeout, const ExpireParams &params, const ResultIntegerCallback &callback) = 0;
    virtual void pexpireat(const std::string &key, int64_t timestamp, const ResultIntegerCallback &callback) = 0;
    virtual void pexpireat(const std::string &key, int64_t timestamp, const ExpireParams &params, const ResultIntegerCallback &callback) = 0;
    virtual void ttl(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void pttl(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void touch(InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void dump(const std::string &key, const ResultStringPtrCallback &callback) = 0;
    virtual void restore(const std::string &key, int64_t ttl, const std::string &value, const ResultStringCallback &callback) = 0;
    virtual void restore(const std::string &key, int64_t ttl, const std::string &value, const RestoreParams &params, const ResultStringCallback &callback) = 0;
    virtual void keys(const std::string &pattern, const ResultVectorStringCallback &callback) = 0;
    virtual void move(const std::string &key, int64_t db, const ResultIntegerCallback &callback) = 0;
    virtual void randomkey(const ResultStringPtrCallback &callback) = 0;
    virtual void rename(const std::string &key, const std::string &newkey, const ResultStringCallback &callback) = 0;
    virtual void renamenx(const std::string &key, const std::string &newkey, const ResultIntegerCallback &callback) = 0;
    virtual void type(const std::string &key, const ResultStringCallback &callback) = 0;
    virtual void scan(const std::string &cursor, const ResultScanCallback &callback) = 0;
    virtual void scan(const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) = 0;
    virtual void sort(const std::string &key, const ResultVectorStringPtrCallback &callback) = 0;
    virtual void sort(const std::string &key, const SortParams &params, const ResultVectorStringPtrCallback &callback) = 0;
    virtual void sortStore(const std::string &key, const std::string &storekey, const ResultIntegerCallback &callback) = 0;
    virtual void sortStore(const std::string &key, const std::string &storekey, const SortParams &params, const ResultIntegerCallback &callback) = 0;
    virtual void copy(const std::string &key, const std::string &destkey, const ResultIntegerCallback &callback) = 0;
    virtual void copy(const std::string &key, const std::string &destkey, const CopyParams &params, const ResultIntegerCallback &callback) = 0;

    // string
    virtual void append(const std::string &key, const std::string &value, const ResultIntegerCallback &callback) = 0;
    virtual void bitcount(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void bitcount(const std::string &key, const BitPositonParams &params, const ResultIntegerCallback &callback) = 0;
    virtual void bitfield(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback) = 0;
    virtual void bitfieldRo(const std::string &key, InitializerList<std::string> args, const ResultVectorIntegerPtrCallback &callback) = 0;
    virtual void bitop(const BitOperation &op, InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void bitpos(const std::string &key, int64_t bit, const ResultIntegerCallback &callback) = 0;
    virtual void bitpos(const std::string &key, int64_t bit, const BitPositonParams &params, const ResultIntegerCallback &callback) = 0;
    virtual void setbit(const std::string &key, int64_t offset, int64_t value, const ResultIntegerCallback &callback) = 0;
    virtual void getbit(const std::string &key, int64_t offset, const ResultIntegerCallback &callback) = 0;
    virtual void decr(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void decrby(const std::string &key, int64_t decrement, const ResultIntegerCallback &callback) = 0;
    virtual void getrange(const std::string &key, int64_t start, int64_t end, const ResultStringCallback &callback) = 0;
    virtual void getset(const std::string &key, const std::string &value, const ResultStringPtrCallback &callback) = 0;
    virtual void set(const std::string &key, const std::string &value, const SetParams &params, const ResultStringCallback &callback) = 0;
    virtual void set(const std::string &key, const std::string &value, const ResultStringCallback &callback) = 0;
    virtual void get(const std::string &key, const ResultStringPtrCallback &callback) = 0;
    virtual void incr(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void incrby(const std::string &key, int64_t increment, const ResultIntegerCallback &callback) = 0;
    virtual void incrbyfloat(const std::string &key, double increment, const ResultStringCallback &callback) = 0;
    virtual void mget(InitializerList<std::string> keys, const ResultVectorStringPtrCallback &callback) = 0;
    virtual void mset(InitializerList<std::string> kvs, const ResultStringCallback &callback) = 0;
    virtual void msetnx(InitializerList<std::string> kvs, const ResultIntegerCallback &callback) = 0;
    virtual void psetex(const std::string &key, int64_t milliseconds, const std::string &value, const ResultStringCallback &callback) = 0;
    virtual void setex(const std::string &key, int64_t seconds, const std::string &value, const ResultStringCallback &callback) = 0;
    virtual void setnx(const std::string &key, const std::string &value, const ResultIntegerCallback &callback) = 0;
    virtual void setrange(const std::string &key, int64_t offset, const std::string &value, const ResultIntegerCallback &callback) = 0;
    virtual void strlen(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void getdel(const std::string &key, const ResultStringPtrCallback &callback) = 0;
    virtual void getex(const std::string &key, const GetExParams &params, const ResultStringPtrCallback &callback) = 0;

    // list
    virtual void blpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) = 0;
    virtual void brpop(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) = 0;
    virtual void brpoplpush(const std::string &src, const std::string &dest, int64_t timeout, const ResultStringPtrCallback &callback) = 0;
    virtual void lindex(const std::string &key, int64_t index, const ResultStringPtrCallback &callback) = 0;
    virtual void linsert(const std::string &key, const ListDirection &direction, const std::string &pivot, const std::string &element, const ResultIntegerCallback &callback) = 0;
    virtual void llen(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void lpop(const std::string &key, const ResultStringPtrCallback &callback) = 0;
    virtual void lpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) = 0;
    virtual void lpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) = 0;
    virtual void lpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) = 0;
    virtual void lrange(const std::string &key, int64_t start, int64_t stop, const ResultVectorStringCallback &callback) = 0;
    virtual void lrem(const std::string &key, int64_t count, const std::string &element, const ResultIntegerCallback &callback) = 0;
    virtual void lset(const std::string &key, int64_t index, const std::string &element, const ResultStringCallback &callback) = 0;
    virtual void ltrim(const std::string &key, int64_t start, int64_t stop, const ResultStringCallback &callback) = 0;
    virtual void rpop(const std::string &key, const ResultStringPtrCallback &callback) = 0;
    virtual void rpop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) = 0;
    virtual void rpoplpush(const std::string &src, const std::string &dest, const ResultStringPtrCallback &callback) = 0;
    virtual void rpush(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) = 0;
    virtual void rpushx(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) = 0;
    virtual void lmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, const ResultStringPtrCallback &callback) = 0;
    virtual void blmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, int64_t timeout, const ResultStringPtrCallback &callback) = 0;

    // set
    virtual void sadd(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) = 0;
    virtual void scard(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void sismember(const std::string &key, const std::string &member, const ResultIntegerCallback &callback) = 0;
    virtual void smembers(const std::string &key, const ResultVectorStringCallback &callback) = 0;
    virtual void smove(const std::string &src, const std::string &dest, const std::string &member, const ResultIntegerCallback &callback) = 0;
    virtual void srem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) = 0;
    virtual void spop(const std::string &key, const ResultStringPtrCallback &callback) = 0;
    virtual void spop(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) = 0;
    virtual void srandmember(const std::string &key, const ResultStringPtrCallback &callback) = 0;
    virtual void srandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) = 0;
    virtual void sdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) = 0;
    virtual void sinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) = 0;
    virtual void sunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) = 0;
    virtual void sdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void sinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void sunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void sscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) = 0;
    virtual void sscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) = 0;
    virtual void smismember(const std::string &key, InitializerList<std::string> members, const ResultVectorIntegerCallback &callback) = 0;

    // hash
    virtual void hdel(const std::string &key, InitializerList<std::string> fields, const ResultIntegerCallback &callback) = 0;
    virtual void hexists(const std::string &key, const std::string &field, const ResultIntegerCallback &callback) = 0;
    virtual void hget(const std::string &key, const std::string &field, const ResultStringPtrCallback &callback) = 0;
    virtual void hgetall(const std::string &key, const ResultVectorStringCallback &callback) = 0;
    virtual void hincrby(const std::string &key, const std::string &field, int64_t increment, const ResultIntegerCallback &callback) = 0;
    virtual void hincrbyfloat(const std::string &key, const std::string &field, double increment, const ResultStringCallback &callback) = 0;
    virtual void hkeys(const std::string &key, const ResultVectorStringCallback &callback) = 0;
    virtual void hvals(const std::string &key, const ResultVectorStringCallback &callback) = 0;
    virtual void hlen(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void hmget(const std::string &key, InitializerList<std::string> fields, const ResultVectorStringPtrCallback &callback) = 0;
    virtual void hset(const std::string &key, const std::string &filed, const std::string &value, const ResultIntegerCallback &callback) = 0;
    virtual void hset(const std::string &key, InitializerList<std::string> kvs, const ResultIntegerCallback &callback) = 0;
    virtual void hsetnx(const std::string &key, const std::string &field, const std::string &value, const ResultIntegerCallback &callback) = 0;
    virtual void hstrlen(const std::string &key, const std::string &field, const ResultIntegerCallback &callback) = 0;
    virtual void hscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) = 0;
    virtual void hscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) = 0;
    virtual void hrandfield(const std::string &key, const ResultStringPtrCallback &callback) = 0;
    virtual void hrandfield(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) = 0;

    // zset
    virtual void zadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) = 0;
    virtual void zadd(const std::string &key, const ZAddParams &params, InitializerList<std::string> elements, const ResultIntegerCallback &callback) = 0;
    virtual void zincrby(const std::string &key, int64_t increment, const std::string &member, const ResultStringCallback &callback) = 0;
    virtual void zscore(const std::string &key, const std::string &member, const ResultStringPtrCallback &callback) = 0;
    virtual void zrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback) = 0;
    virtual void zrevrank(const std::string &key, const std::string &member, const ResultIntegerPtrCallback &callback) = 0;
    virtual void zcard(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void zcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) = 0;
    virtual void zlexcount(const std::string &key, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) = 0;
    virtual void zpopmax(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) = 0;
    virtual void zpopmin(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) = 0;
    virtual void bzpopmax(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) = 0;
    virtual void bzpopmin(InitializerList<std::string> keys, int64_t timeout, const ResultVectorStringCallback &callback) = 0;
    virtual void zrange(const std::string &key, const std::string &start, const std::string &stop, const ResultVectorStringCallback &callback) = 0;
    virtual void zrange(const std::string &key, const std::string &start, const std::string &stop, const ZRangeParams &params, const ResultVectorStringCallback &callback) = 0;
    virtual void zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ResultIntegerCallback &callback) = 0;
    virtual void zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ZRangeParams &params, const ResultIntegerCallback &callback) = 0;
    virtual void zrem(const std::string &key, InitializerList<std::string> members, const ResultIntegerCallback &callback) = 0;
    virtual void zremrange(const ZRemRangeOption &option, const std::string &key, const std::string &begin, const std::string &end, const ResultIntegerCallback &callback) = 0;
    virtual void zinter(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) = 0;
    virtual void zinter(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback) = 0;
    virtual void zinterstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void zinterstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback) = 0;
    virtual void zunion(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) = 0;
    virtual void zunion(InitializerList<std::string> keys, ZInterUnionParams &params, const ResultVectorStringCallback &callback) = 0;
    virtual void zunionstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void zunionstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params, const ResultIntegerCallback &callback) = 0;
    virtual void zdiff(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) = 0;
    virtual void zdiffWithScores(InitializerList<std::string> keys, const ResultVectorStringCallback &callback) = 0;
    virtual void zdiffstore(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void zdiffstoreWithScores(const std::string &dest, InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void zmscore(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback) = 0;
    virtual void zrandmember(const std::string &key, const ResultStringPtrCallback &callback) = 0;
    virtual void zrandmember(const std::string &key, int64_t count, const ResultVectorStringCallback &callback) = 0;
    virtual void zscan(const std::string &key, const std::string &cursor, const ResultScanCallback &callback) = 0;
    virtual void zscan(const std::string &key, const std::string &cursor, const ScanParams &params, const ResultScanCallback &callback) = 0;

    // hyperloglog
    virtual void pfadd(const std::string &key, InitializerList<std::string> elements, const ResultIntegerCallback &callback) = 0;
    virtual void pfcount(InitializerList<std::string> keys, const ResultIntegerCallback &callback) = 0;
    virtual void pfmerge(const std::string &dest, InitializerList<std::string> keys, const ResultStringCallback &callback) = 0;

    // geo
    virtual void geoadd(const std::string &key, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback) = 0;
    virtual void geoadd(const std::string &key, const GeoAddParams &params, const std::vector<std::tuple<double, double, std::string>> &members, const ResultIntegerCallback &callback) = 0;
    virtual void geodist(const std::string &key, const std::string &member1, const std::string &member2, const GeoUnit &unit, const ResultStringPtrCallback &callback) = 0;
    virtual void geohash(const std::string &key, InitializerList<std::string> members, const ResultVectorStringPtrCallback &callback) = 0;
    virtual void geopos(const std::string &key, InitializerList<std::string> members, const ResultGeoposCallback &callback) = 0;
    virtual void georadius(const std::string &key, double longitude, double latitude, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback) = 0;
    virtual void georadiusbymember(const std::string &key, const std::string &member, double radius, const GeoUnit &unit, const ResultVectorStringCallback &callback) = 0;

    // stream
    virtual void xadd(const std::string &key, const std::string &id, InitializerList<std::string> elements, const ResultStringCallback &callback) = 0;
    virtual void xlen(const std::string &key, const ResultIntegerCallback &callback) = 0;
    virtual void xdel(const std::string &key, InitializerList<std::string> ids, const ResultIntegerCallback &callback) = 0;
    virtual void xack(const std::string &key, const std::string &group, InitializerList<std::string> ids, const ResultIntegerCallback &callback) = 0;
    virtual void xgroupCreate(const std::string &key, const std::string &group, const std::string &id, const ResultStringCallback &callback) = 0;
    virtual void xgroupCreateConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback) = 0;
    virtual void xgroupDelConsumer(const std::string &key, const std::string &group, const std::string &consumer, const ResultIntegerCallback &callback) = 0;
    virtual void xgroupDestroy(const std::string &key, const std::string &group, const ResultIntegerCallback &callback) = 0;
    virtual void xgroupSetID(const std::string &key, const std::string &group, const ResultStringCallback &callback) = 0;
    virtual void xpending(const std::string &key, const std::string &group, const ResultXPendingCallback &callback) = 0;
    virtual void xrange(const std::string &key, const std::string &start, const std::string &end, const ResultXRangeCallback &callback) = 0;
    virtual void xrevrange(const std::string &key, const std::string &end, const std::string &start, const ResultXRangeCallback &callback) = 0;
    virtual void xread(int64_t count, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback) = 0;
    virtual void xreadgroup(const std::string &group, const std::string &consumer, InitializerList<std::string> keys, InitializerList<std::string> ids, const ResultXreadCallback &callback) = 0;
    virtual void xtrim(const std::string &key, const std::string &strategy, int64_t threshold, const ResultIntegerCallback &callback) = 0;
    virtual void xsetid(const std::string &key, const std::string &last_id, const ResultStringCallback &callback) = 0;

    // script
    virtual void eval(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) = 0;
    virtual void evalRo(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) = 0;
    virtual void evalsha(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) = 0;
    virtual void evalshaRo(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args, const ResultPacketCallback &callback) = 0;
    virtual void scriptLoad(const std::string &script, const ResultStringCallback &callback) = 0;
    virtual void scriptFlush(const ResultStringCallback &callback) = 0;
    virtual void scriptKill(const ResultStringCallback &callback) = 0;
    virtual void scriptExists(InitializerList<std::string> sha1s, const ResultVectorIntegerCallback &callback) = 0;

    // connection
    virtual void auth(const std::string &password, const ResultStringCallback &callback) = 0;
    virtual void auth(const std::string &user, const std::string &password, const ResultStringCallback &callback) = 0;
    virtual void quit(const ResultStringCallback &callback) = 0;

    // server
    virtual void flushall(const ResultStringCallback &callback) = 0;

    // pubsub
    virtual void publish(const std::string &channel, const std::string &message, const ResultIntegerCallback &callback) = 0;
    virtual void clusterPublish(const std::string &channel, const std::string &message, const std::string &name, int flag, const ResultIntegerCallback &callback) = 0;

    // cluster
    virtual void clusterNodes(const ResultStringCallback &callback) = 0;
};

} // namespace tair::client
