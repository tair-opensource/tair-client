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

#include <future>

#include "client/TairClientDefine.hpp"
#include "client/params/ParamsAll.hpp"

namespace tair::client {

class TairClient;

class TairClientWrapper {
    friend class TairClient;

private:
    TairClientWrapper(TairClient &client)
        : client_(client) {}

public:
    ~TairClientWrapper() = default;

    // -------------------------------- send Command --------------------------------
    std::future<PacketPtr> sendCommand(CommandArgv &&argv);
    std::future<PacketPtr> sendCommand(const CommandArgv &argv);

    // -------------------------------- Generic Command --------------------------------
    std::future<TairResult<int64_t>> del(const std::string &key);
    std::future<TairResult<int64_t>> del(InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> unlink(const std::string &key);
    std::future<TairResult<int64_t>> unlink(InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> exists(const std::string &key);
    std::future<TairResult<int64_t>> exists(InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> expire(const std::string &key, int64_t timeout);
    std::future<TairResult<int64_t>> expire(const std::string &key, int64_t timeout, const ExpireParams &params);
    std::future<TairResult<int64_t>> expireat(const std::string &key, int64_t timestamp);
    std::future<TairResult<int64_t>> expireat(const std::string &key, int64_t timestamp, const ExpireParams &params);
    std::future<TairResult<int64_t>> persist(const std::string &key);
    std::future<TairResult<int64_t>> pexpire(const std::string &key, int64_t timeout);
    std::future<TairResult<int64_t>> pexpire(const std::string &key, int64_t timeout, const ExpireParams &params);
    std::future<TairResult<int64_t>> pexpireat(const std::string &key, int64_t timestamp);
    std::future<TairResult<int64_t>> pexpireat(const std::string &key, int64_t timestamp, const ExpireParams &params);
    std::future<TairResult<int64_t>> ttl(const std::string &key);
    std::future<TairResult<int64_t>> pttl(const std::string &key);
    std::future<TairResult<int64_t>> touch(InitializerList<std::string> keys);
    std::future<TairResult<std::shared_ptr<std::string>>> dump(const std::string &key);
    std::future<TairResult<std::string>> restore(const std::string &key, int64_t ttl, const std::string &value);
    std::future<TairResult<std::string>> restore(const std::string &key, int64_t ttl, const std::string &value, const RestoreParams &params);
    std::future<TairResult<std::vector<std::string>>> keys(const std::string &pattern);
    std::future<TairResult<int64_t>> move(const std::string &key, int64_t db);
    std::future<TairResult<std::shared_ptr<std::string>>> randomkey();
    std::future<TairResult<std::string>> rename(const std::string &key, const std::string &newkey);
    std::future<TairResult<int64_t>> renamenx(const std::string &key, const std::string &newkey);
    std::future<TairResult<std::string>> type(const std::string &key);
    std::future<TairResult<ScanResult>> scan(const std::string &cursor);
    std::future<TairResult<ScanResult>> scan(const std::string &cursor, const ScanParams &params);
    std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> sort(const std::string &key);
    std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> sort(const std::string &key, const SortParams &params);
    std::future<TairResult<int64_t>> sortStore(const std::string &key, const std::string &storekey);
    std::future<TairResult<int64_t>> sortStore(const std::string &key, const std::string &storekey, const SortParams &params);
    std::future<TairResult<int64_t>> copy(const std::string &key, const std::string &destkey);
    std::future<TairResult<int64_t>> copy(const std::string &key, const std::string &destkey, const CopyParams &params);

    // -------------------------------- String Command --------------------------------
    std::future<TairResult<int64_t>> append(const std::string &key, const std::string &value);
    std::future<TairResult<int64_t>> bitcount(const std::string &key);
    std::future<TairResult<int64_t>> bitcount(const std::string &key, const BitPositonParams &params);
    std::future<TairResult<std::vector<std::shared_ptr<int64_t>>>> bitfield(const std::string &key, InitializerList<std::string> args);
    std::future<TairResult<std::vector<std::shared_ptr<int64_t>>>> bitfieldRo(const std::string &key, InitializerList<std::string> args);
    std::future<TairResult<int64_t>> bitop(const BitOperation &op, InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> bitpos(const std::string &key, int64_t bit);
    std::future<TairResult<int64_t>> bitpos(const std::string &key, int64_t bit, const BitPositonParams &params);
    std::future<TairResult<int64_t>> setbit(const std::string &key, int64_t offset, int64_t value);
    std::future<TairResult<int64_t>> getbit(const std::string &key, int64_t offset);
    std::future<TairResult<int64_t>> decr(const std::string &key);
    std::future<TairResult<int64_t>> decrby(const std::string &key, int64_t decrement);
    std::future<TairResult<std::string>> getrange(const std::string &key, int64_t start, int64_t end);
    std::future<TairResult<std::shared_ptr<std::string>>> getset(const std::string &key, const std::string &value);
    std::future<TairResult<std::string>> set(const std::string &key, const std::string &value, const SetParams &params);
    std::future<TairResult<std::string>> set(const std::string &key, const std::string &value);
    std::future<TairResult<std::shared_ptr<std::string>>> get(const std::string &key);
    std::future<TairResult<int64_t>> incr(const std::string &key);
    std::future<TairResult<int64_t>> incrby(const std::string &key, int64_t increment);
    std::future<TairResult<std::string>> incrbyfloat(const std::string &key, double increment);
    std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> mget(InitializerList<std::string> keys);
    std::future<TairResult<std::string>> mset(InitializerList<std::string> kvs);
    std::future<TairResult<int64_t>> msetnx(InitializerList<std::string> kvs);
    std::future<TairResult<std::string>> psetex(const std::string &key, int64_t milliseconds, const std::string &value);
    std::future<TairResult<std::string>> setex(const std::string &key, int64_t seconds, const std::string &value);
    std::future<TairResult<int64_t>> setnx(const std::string &key, const std::string &value);
    std::future<TairResult<int64_t>> setrange(const std::string &key, int64_t offset, const std::string &value);
    std::future<TairResult<int64_t>> strlen(const std::string &key);
    std::future<TairResult<std::shared_ptr<std::string>>> getdel(const std::string &key);
    std::future<TairResult<std::shared_ptr<std::string>>> getex(const std::string &key, const GetExParams &params);

    // -------------------------------- Set Command --------------------------------
    std::future<TairResult<int64_t>> sadd(const std::string &key, InitializerList<std::string> members);
    std::future<TairResult<int64_t>> scard(const std::string &key);
    std::future<TairResult<int64_t>> sismember(const std::string &key, const std::string &member);
    std::future<TairResult<std::vector<std::string>>> smembers(const std::string &key);
    std::future<TairResult<int64_t>> smove(const std::string &src, const std::string &dest, const std::string &member);
    std::future<TairResult<int64_t>> srem(const std::string &key, InitializerList<std::string> members);
    std::future<TairResult<std::shared_ptr<std::string>>> spop(const std::string &key);
    std::future<TairResult<std::vector<std::string>>> spop(const std::string &key, int64_t count);
    std::future<TairResult<std::shared_ptr<std::string>>> srandmember(const std::string &key);
    std::future<TairResult<std::vector<std::string>>> srandmember(const std::string &key, int64_t count);
    std::future<TairResult<std::vector<std::string>>> sdiff(InitializerList<std::string> keys);
    std::future<TairResult<std::vector<std::string>>> sinter(InitializerList<std::string> keys);
    std::future<TairResult<std::vector<std::string>>> sunion(InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> sdiffstore(const std::string &dest, InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> sinterstore(const std::string &dest, InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> sunionstore(const std::string &dest, InitializerList<std::string> keys);
    std::future<TairResult<ScanResult>> sscan(const std::string &key, const std::string &cursor);
    std::future<TairResult<ScanResult>> sscan(const std::string &key, const std::string &cursor, const ScanParams &params);
    std::future<TairResult<std::vector<int64_t>>> smismember(const std::string &key, InitializerList<std::string> members);

    // -------------------------------- Hash Command --------------------------------
    std::future<TairResult<int64_t>> hdel(const std::string &key, InitializerList<std::string> fields);
    std::future<TairResult<int64_t>> hexists(const std::string &key, const std::string &field);
    std::future<TairResult<std::shared_ptr<std::string>>> hget(const std::string &key, const std::string &field);
    std::future<TairResult<std::vector<std::string>>> hgetall(const std::string &key);
    std::future<TairResult<int64_t>> hincrby(const std::string &key, const std::string &field, int64_t increment);
    std::future<TairResult<std::string>> hincrbyfloat(const std::string &key, const std::string &field, double increment);
    std::future<TairResult<std::vector<std::string>>> hkeys(const std::string &key);
    std::future<TairResult<std::vector<std::string>>> hvals(const std::string &key);
    std::future<TairResult<int64_t>> hlen(const std::string &key);
    std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> hmget(const std::string &key, InitializerList<std::string> fields);
    std::future<TairResult<int64_t>> hset(const std::string &key, const std::string &filed, const std::string &value);
    std::future<TairResult<int64_t>> hset(const std::string &key, InitializerList<std::string> kvs);
    std::future<TairResult<int64_t>> hsetnx(const std::string &key, const std::string &field, const std::string &value);
    std::future<TairResult<int64_t>> hstrlen(const std::string &key, const std::string &field);
    std::future<TairResult<ScanResult>> hscan(const std::string &key, const std::string &cursor);
    std::future<TairResult<ScanResult>> hscan(const std::string &key, const std::string &cursor, const ScanParams &params);
    std::future<TairResult<std::shared_ptr<std::string>>> hrandfield(const std::string &key);
    std::future<TairResult<std::vector<std::string>>> hrandfield(const std::string &key, int64_t count);

    // -------------------------------- Connection Command --------------------------------
    std::future<TairResult<std::vector<std::string>>> blpop(InitializerList<std::string> keys, int64_t timeout);
    std::future<TairResult<std::vector<std::string>>> brpop(InitializerList<std::string> keys, int64_t timeout);
    std::future<TairResult<std::shared_ptr<std::string>>> brpoplpush(const std::string &src, const std::string &dest, int64_t timeout);
    std::future<TairResult<std::shared_ptr<std::string>>> lindex(const std::string &key, int64_t index);
    std::future<TairResult<int64_t>> linsert(const std::string &key, const ListDirection &direction, const std::string &pivot, const std::string &element);
    std::future<TairResult<int64_t>> llen(const std::string &key);
    std::future<TairResult<std::shared_ptr<std::string>>> lpop(const std::string &key);
    std::future<TairResult<std::vector<std::string>>> lpop(const std::string &key, int64_t count);
    std::future<TairResult<int64_t>> lpush(const std::string &key, InitializerList<std::string> elements);
    std::future<TairResult<int64_t>> lpushx(const std::string &key, InitializerList<std::string> elements);
    std::future<TairResult<std::vector<std::string>>> lrange(const std::string &key, int64_t start, int64_t stop);
    std::future<TairResult<int64_t>> lrem(const std::string &key, int64_t count, const std::string &element);
    std::future<TairResult<std::string>> lset(const std::string &key, int64_t index, const std::string &element);
    std::future<TairResult<std::string>> ltrim(const std::string &key, int64_t start, int64_t stop);
    std::future<TairResult<std::shared_ptr<std::string>>> rpop(const std::string &key);
    std::future<TairResult<std::vector<std::string>>> rpop(const std::string &key, int64_t count);
    std::future<TairResult<std::shared_ptr<std::string>>> rpoplpush(const std::string &src, const std::string &dest);
    std::future<TairResult<int64_t>> rpush(const std::string &key, InitializerList<std::string> elements);
    std::future<TairResult<int64_t>> rpushx(const std::string &key, InitializerList<std::string> elements);
    std::future<TairResult<std::shared_ptr<std::string>>> lmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd);
    std::future<TairResult<std::shared_ptr<std::string>>> blmove(const std::string &src, const std::string &dest, const ListDirection &ld, const ListDirection &rd, int64_t timeout);

    // -------------------------------- Zset Command --------------------------------
    std::future<TairResult<int64_t>> zadd(const std::string &key, InitializerList<std::string> elements);
    std::future<TairResult<int64_t>> zadd(const std::string &key, const ZAddParams &params, InitializerList<std::string> elements);
    std::future<TairResult<std::string>> zincrby(const std::string &key, int64_t increment, const std::string &member);
    std::future<TairResult<std::shared_ptr<std::string>>> zscore(const std::string &key, const std::string &member);
    std::future<TairResult<std::shared_ptr<int64_t>>> zrank(const std::string &key, const std::string &member);
    std::future<TairResult<std::shared_ptr<int64_t>>> zrevrank(const std::string &key, const std::string &member);
    std::future<TairResult<int64_t>> zcard(const std::string &key);
    std::future<TairResult<int64_t>> zcount(const std::string &key, const std::string &min, const std::string &max);
    std::future<TairResult<int64_t>> zlexcount(const std::string &key, const std::string &min, const std::string &max);
    std::future<TairResult<std::vector<std::string>>> zpopmax(const std::string &key, int64_t count);
    std::future<TairResult<std::vector<std::string>>> zpopmin(const std::string &key, int64_t count);
    std::future<TairResult<std::vector<std::string>>> bzpopmax(InitializerList<std::string> keys, int64_t timeout);
    std::future<TairResult<std::vector<std::string>>> bzpopmin(InitializerList<std::string> keys, int64_t timeout);
    std::future<TairResult<std::vector<std::string>>> zrange(const std::string &key, const std::string &start, const std::string &stop);
    std::future<TairResult<std::vector<std::string>>> zrange(const std::string &key, const std::string &start, const std::string &stop, const ZRangeParams &params);
    std::future<TairResult<int64_t>> zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max);
    std::future<TairResult<int64_t>> zrangestore(const std::string &dest, const std::string &src, const std::string &min, const std::string &max, const ZRangeParams &params);
    std::future<TairResult<int64_t>> zrem(const std::string &key, InitializerList<std::string> members);
    std::future<TairResult<int64_t>> zremrange(const ZRemRangeOption &option, const std::string &key, const std::string &begin, const std::string &end);
    std::future<TairResult<std::vector<std::string>>> zinter(InitializerList<std::string> keys);
    std::future<TairResult<std::vector<std::string>>> zinter(InitializerList<std::string> keys, ZInterUnionParams &params);
    std::future<TairResult<int64_t>> zinterstore(const std::string &dest, InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> zinterstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params);
    std::future<TairResult<std::vector<std::string>>> zunion(InitializerList<std::string> keys);
    std::future<TairResult<std::vector<std::string>>> zunion(InitializerList<std::string> keys, ZInterUnionParams &params);
    std::future<TairResult<int64_t>> zunionstore(const std::string &dest, InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> zunionstore(const std::string &dest, InitializerList<std::string> keys, ZInterUnionParams &params);
    std::future<TairResult<std::vector<std::string>>> zdiff(InitializerList<std::string> keys);
    std::future<TairResult<std::vector<std::string>>> zdiffWithScores(InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> zdiffstore(const std::string &dest, InitializerList<std::string> keys);
    std::future<TairResult<int64_t>> zdiffstoreWithScores(const std::string &dest, InitializerList<std::string> keys);
    std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> zmscore(const std::string &key, InitializerList<std::string> members);
    std::future<TairResult<std::shared_ptr<std::string>>> zrandmember(const std::string &key);
    std::future<TairResult<std::vector<std::string>>> zrandmember(const std::string &key, int64_t count);
    std::future<TairResult<ScanResult>> zscan(const std::string &key, const std::string &cursor);
    std::future<TairResult<ScanResult>> zscan(const std::string &key, const std::string &cursor, const ScanParams &params);

    // -------------------------------- HyperLogLog Command --------------------------------
    std::future<TairResult<int64_t>> pfadd(const std::string &key, InitializerList<std::string> elements);
    std::future<TairResult<int64_t>> pfcount(InitializerList<std::string> keys);
    std::future<TairResult<std::string>> pfmerge(const std::string &dest, InitializerList<std::string> keys);

    // -------------------------------- Geo Command --------------------------------
    std::future<TairResult<int64_t>> geoadd(const std::string &key, const std::vector<std::tuple<double, double, std::string>> &members);
    std::future<TairResult<int64_t>> geoadd(const std::string &key, const GeoAddParams &params, const std::vector<std::tuple<double, double, std::string>> &members);
    std::future<TairResult<std::shared_ptr<std::string>>> geodist(const std::string &key, const std::string &member1, const std::string &member2, const GeoUnit &unit);
    std::future<TairResult<std::vector<std::shared_ptr<std::string>>>> geohash(const std::string &key, InitializerList<std::string> members);
    std::future<TairResult<GeoPosResult>> geopos(const std::string &key, InitializerList<std::string> members);
    std::future<TairResult<std::vector<std::string>>> georadius(const std::string &key, double longitude, double latitude, double radius, const GeoUnit &unit);
    std::future<TairResult<std::vector<std::string>>> georadiusbymember(const std::string &key, const std::string &member, double radius, const GeoUnit &unit);

    // -------------------------------- Stream Command --------------------------------
    std::future<TairResult<std::string>> xadd(const std::string &key, const std::string &id, InitializerList<std::string> elements);
    std::future<TairResult<int64_t>> xlen(const std::string &key);
    std::future<TairResult<int64_t>> xdel(const std::string &key, InitializerList<std::string> ids);
    std::future<TairResult<int64_t>> xack(const std::string &key, const std::string &group, InitializerList<std::string> ids);
    std::future<TairResult<std::string>> xgroupCreate(const std::string &key, const std::string &group, const std::string &id);
    std::future<TairResult<int64_t>> xgroupCreateConsumer(const std::string &key, const std::string &group, const std::string &consumer);
    std::future<TairResult<int64_t>> xgroupDelConsumer(const std::string &key, const std::string &group, const std::string &consumer);
    std::future<TairResult<int64_t>> xgroupDestroy(const std::string &key, const std::string &group);
    std::future<TairResult<std::string>> xgroupSetID(const std::string &key, const std::string &group);
    std::future<TairResult<XPendingResult>> xpending(const std::string &key, const std::string &group);
    std::future<TairResult<std::vector<XRangeResult>>> xrange(const std::string &key, const std::string &start, const std::string &end);
    std::future<TairResult<std::vector<XRangeResult>>> xrevrange(const std::string &key, const std::string &end, const std::string &start);
    std::future<TairResult<std::vector<XReadResult>>> xread(int64_t count, InitializerList<std::string> keys, InitializerList<std::string> ids);
    std::future<TairResult<std::vector<XReadResult>>> xreadgroup(const std::string &group, const std::string &consumer, InitializerList<std::string> keys, InitializerList<std::string> ids);
    std::future<TairResult<int64_t>> xtrim(const std::string &key, const std::string &strategy, int64_t threshold);
    std::future<TairResult<std::string>> xsetid(const std::string &key, const std::string &last_id);

    // -------------------------------- Script Command --------------------------------
    std::future<PacketPtr> eval(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args);
    std::future<PacketPtr> evalRo(const std::string &script, InitializerList<std::string> keys, InitializerList<std::string> args);
    std::future<PacketPtr> evalsha(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args);
    std::future<PacketPtr> evalshaRo(const std::string &sha1, InitializerList<std::string> keys, InitializerList<std::string> args);
    std::future<TairResult<std::string>> scriptLoad(const std::string &script);
    std::future<TairResult<std::string>> scriptFlush();
    std::future<TairResult<std::string>> scriptKill();
    std::future<TairResult<std::vector<int64_t>>> scriptExists(InitializerList<std::string> sha1s);

    // -------------------------------- Connection Command --------------------------------
    std::future<TairResult<std::string>> auth(const std::string &password);
    std::future<TairResult<std::string>> auth(const std::string &user, const std::string &password);
    std::future<TairResult<std::string>> quit();

    // -------------------------------- Server Command --------------------------------
    std::future<TairResult<std::string>> flushall();

    // -------------------------------- Pubsub Command --------------------------------
    std::future<TairResult<int64_t>> publish(const std::string &channel, const std::string &message);
    std::future<TairResult<int64_t>> clusterPublish(const std::string &channel, const std::string &message, const std::string &name, int flag);

    // -------------------------------- Cluster Command --------------------------------
    std::future<TairResult<std::string>> clusterNodes();

private:
    TairClient &client_;
};

} // namespace tair::client
