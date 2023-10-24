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
#include "client/TairSubscribeClient.hpp"

#include "network/EventLoopThread.hpp"
#include "protocol/packet/resp/RESPPacketHelper.hpp"

namespace tair::client {

using protocol::RESPPacketHelper;

void TairSubscribeClient::onConnected() {
    TairBaseClient::onConnected();
    runtimeAssert(loop_->isInLoopThread());
    for (const auto &pair : sub_callbacks_) {
        subscribe(pair.first, nullptr, pair.second);
    }
    for (const auto &pair : psub_callbacks_) {
        psubscribe(pair.first, nullptr, pair.second);
    }
}

void TairSubscribeClient::doSubscribeMessage(const PacketPtr &resp) {
    std::string type, channel, message;
    if (RESPPacketHelper::getReplyBulkStrs(resp.get(), type, channel, message)) {
        runtimeAssert(type == "message");
        auto pair = sub_callbacks_.find(channel);
        if (pair != sub_callbacks_.end() && pair->second) {
            pair->second(SubMessage(channel, message));
        }
    }
}

void TairSubscribeClient::doPSubscribeMessage(const PacketPtr &resp) {
    std::string type, pattern, channel, message;
    if (RESPPacketHelper::getReplyBulkStrs(resp.get(), type, pattern, channel, message)) {
        runtimeAssert(type == "pmessage");
        auto pair = psub_callbacks_.find(pattern);
        if (pair != psub_callbacks_.end() && pair->second) {
            pair->second(PSubMessage(pattern, channel, message));
        }
    }
}

void TairSubscribeClient::onRecvResponse(const PacketPtr &resp) {
    std::string type;
    auto ret = RESPPacketHelper::getReplyArrayFirstBulkStr(resp.get(), type);
    if (ret && (type == "message" || type == "pmessage")) {
        if (type == "message") { // subscribe message
            doSubscribeMessage(resp);
        } else if (type == "pmessage") { // psubscribe message
            doPSubscribeMessage(resp);
        }
    } else {
        runtimeAssert(!callbacks_.empty());
        // auth、subscribe、unsubscribe、psubscribe、 punsubscribe or error
        auto [init_time, req, callback] = callbacks_.front();
        int64_t latency_us = ClockTime::intervalUs() - init_time;
        callback(req, resp, latency_us);
        callbacks_.pop_front();
    }
}

void TairSubscribeClient::doPubSubCount(const PacketPtr &resp, const ResultPubSubCountCallback &callback,
                                        const SubMessageCallback &msg_callback, const PSubMessageCallback &pmsg_callback) {
    std::string type, channel;
    int64_t count = 0;
    auto ret = RESPPacketHelper::getReplyBulkBulkInteger(resp.get(), type, channel, count);
    runtimeAssert(ret);
    if (type == "subscribe") {
        sub_callbacks_[channel] = msg_callback;
    } else if (type == "unsubscribe") {
        sub_callbacks_.erase(channel);
    } else if (type == "psubscribe") {
        psub_callbacks_[channel] = pmsg_callback;
    } else if (type == "punsubscribe") {
        psub_callbacks_.erase(channel);
    }
    auto result = TairResult<PubSubCount>::create(PubSubCount(type, channel, count));
    if (callback) {
        callback(result);
    }
}

void TairSubscribeClient::doPubSubResponse(const PacketPtr &resp, const ResultPubSubCountCallback &callback,
                                           const SubMessageCallback &msg_callback, const PSubMessageCallback &pmsg_callback) {
    size_t array_size;
    if (RESPPacketHelper::getReplyArraySize(resp.get(), array_size) && array_size == 3) {
        doPubSubCount(resp, callback, msg_callback, pmsg_callback);
    } else {
        TairResult<PubSubCount> result;
        std::string error;
        if (RESPPacketHelper::getReplyError(resp.get(), error)) {
            result.setErr(error);
        } else {
            result.setErr("unknown pub/sub response type");
        }
        if (callback) {
            callback(result);
        }
    }
}

void TairSubscribeClient::subscribe(const std::string &channel,
                                    const ResultPubSubCountCallback &callback, const SubMessageCallback &msg_callback) {
    sendCommand({"subscribe", channel}, [this, callback, msg_callback](auto &, auto &resp, int64_t) {
        doPubSubResponse(resp, callback, msg_callback, nullptr);
    });
}

void TairSubscribeClient::unSubscribe(const std::string &channel, const ResultPubSubCountCallback &packet) {
    sendCommand({"unsubscribe", channel}, [this, packet](auto &, auto &resp, int64_t) {
        doPubSubResponse(resp, packet, nullptr, nullptr);
    });
}

void TairSubscribeClient::psubscribe(const std::string &pattern,
                                     const ResultPubSubCountCallback &callback, const PSubMessageCallback &pmsg_callback) {
    sendCommand({"psubscribe", pattern}, [this, callback, pmsg_callback](auto &, auto &resp, int64_t) {
        doPubSubResponse(resp, callback, nullptr, pmsg_callback);
    });
}

void TairSubscribeClient::unPsubscribe(const std::string &pattern, const ResultPubSubCountCallback &callback) {
    sendCommand({"punsubscribe", pattern}, [this, callback](auto &, auto &resp, int64_t) {
        doPubSubResponse(resp, callback, nullptr, nullptr);
    });
}

} // namespace tair::client
