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

#include <unordered_map>

#include "client/TairBaseClient.hpp"

namespace tair::client {

struct SubMessage {
    SubMessage(const std::string &ch, const std::string &msg)
        : channel(ch), message(msg) {}
    std::string channel;
    std::string message;
};

struct PSubMessage {
    PSubMessage(const std::string &pt, const std::string &ch, const std::string &msg)
        : pattern(pt), channel(ch), message(msg) {}
    std::string pattern;
    std::string channel;
    std::string message;
};

struct PubSubCount {
    PubSubCount()
        : count(0) {}
    PubSubCount(const std::string &t, const std::string &n, int64_t c)
        : type(t), name(n), count(c) {}
    std::string type; // subscribe、unsubscribe、psubscribe or punsubscribe
    std::string name; // channel or pattern name
    int64_t count;
};

using ResultPubSubCountCallback = std::function<void(const TairResult<PubSubCount> &)>;
using SubMessageCallback = std::function<void(const SubMessage &)>;
using PSubMessageCallback = std::function<void(const PSubMessage &)>;

class TairSubscribeClient : public TairBaseClient {
public:
    TairSubscribeClient()
        : TairBaseClient() {}
    TairSubscribeClient(EventLoop *loop)
        : TairBaseClient(loop) {}
    ~TairSubscribeClient() override {
        disconnect();
    }

private:
    void onConnected() override;

    void doSubscribeMessage(const PacketPtr &resp);
    void doPSubscribeMessage(const PacketPtr &resp);
    void doPubSubCount(const PacketPtr &resp, const ResultPubSubCountCallback &callback,
                       const SubMessageCallback &msg_callback, const PSubMessageCallback &pmsg_callback);
    void doPubSubResponse(const PacketPtr &resp, const ResultPubSubCountCallback &callback,
                          const SubMessageCallback &msg_callback, const PSubMessageCallback &pmsg_callback);

    void onRecvResponse(const PacketPtr &resp) override;

public:
    void subscribe(const std::string &channel,
                   const ResultPubSubCountCallback &callback, const SubMessageCallback &msg_callback);
    void unSubscribe(const std::string &channel, const ResultPubSubCountCallback &callback);

    void psubscribe(const std::string &channel,
                    const ResultPubSubCountCallback &callback, const PSubMessageCallback &pmsg_callback);
    void unPsubscribe(const std::string &pattern, const ResultPubSubCountCallback &callback);

private:
    std::unordered_map<std::string, SubMessageCallback> sub_callbacks_;
    std::unordered_map<std::string, PSubMessageCallback> psub_callbacks_;
};

} //  namespace tair::client
