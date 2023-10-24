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

#include "common/Logger.hpp"
#include "protocol/packet/resp/ArrayPacket.hpp"
#include "protocol/packet/resp/RESPPacketHelper.hpp"
#include "client/TairClientDefine.hpp"
#include "client/TairResult.hpp"
#include "client/results/ResultsAll.hpp"

namespace tair::client {

using protocol::RESPPacketHelper;
using protocol::PacketPtr;
using protocol::ArrayPacket;
using protocol::BulkStringPacket;
using protocol::IntegerPacket;
using protocol::NullPacket;
using protocol::PacketType;

class TairResultHelper {
public:
    template <typename PACKET_TYPE, typename VALUE, typename C>
    static void doCallbackOneResult(const PacketPtr &resp, const C &callback) {
        TairResult<VALUE> result;
        if (resp) {
            VALUE value;
            if (RESPPacketHelper::getReplyData<PACKET_TYPE>(resp.get(), value)) {
                result.setValue(value);
            } else {
                std::string error;
                if (RESPPacketHelper::getReplyError(resp.get(), error)) {
                    result.setErr(error);
                } else {
                    result.setErr("unknown return type");
                }
            }
        } else {
            result.setErr("connection error, response is null");
        }
        callback(result);
    }

    static void vectorStringBuilder(ArrayPacket *ap, TairResult<std::vector<std::string>> &result) {
        std::vector<std::string> bulks;
        bool ret = ap->moveBulks(bulks);
        if (!ret) {
            result.setErr("FATAL: decode vector string failed.");
            return;
        }
        result.setValue(bulks);
    }

    static void vectorIntegerBuilder(ArrayPacket *ap, TairResult<std::vector<int64_t>> &result) {
        std::vector<int64_t> integers;
        bool ret = ap->moveIntegers(integers);
        if (!ret) {
            result.setErr("FATAL: decode vector integer failed.");
            return;
        }
        result.setValue(integers);
    }

    static void vectorIntegerPtrBuilder(ArrayPacket *ap, TairResult<std::vector<std::shared_ptr<int64_t>>> &result) {
        std::vector<std::shared_ptr<int64_t>> v_ptr;
        auto packet_array = ap->getPacketArray();
        for (auto packet : packet_array) {
            IntegerPacket *integer_packet = packet->packet_cast<IntegerPacket>();
            if (integer_packet) {
                v_ptr.push_back(std::make_shared<int64_t>(integer_packet->getValue()));
            } else {
                // Notice: Although addReplyNull is called when the server returns,
                // but in ArrayPacket::decode, by judging `$`, BulkStringPacket::decode will be called
                // That is, NullPacket is just a special type of BulkStringPacket.
                BulkStringPacket *bulk_packet = packet->packet_cast<BulkStringPacket>();
                runtimeAssert(bulk_packet->getType() == PacketType::TYPE_NULL);
                v_ptr.push_back(nullptr);
            }
        }
        result.setValue(v_ptr);
    }

    static void vectorStringPtrBuilder(ArrayPacket *ap, TairResult<std::vector<std::shared_ptr<std::string>>> &result) {
        std::vector<std::shared_ptr<std::string>> v_ptr;
        auto packet_array = ap->getPacketArray();
        for (auto packet : packet_array) {
            BulkStringPacket *bulk_packet = packet->packet_cast<BulkStringPacket>();
            runtimeAssert(bulk_packet);
            if (bulk_packet->getType() != PacketType::TYPE_NULL) {
                v_ptr.push_back(std::make_shared<std::string>(bulk_packet->getValue()));
            } else {
                v_ptr.push_back(nullptr);
            }
        }
        result.setValue(v_ptr);
    }

    static void stringPtrBuilder(BulkStringPacket *packet, TairResult<std::shared_ptr<std::string>> &result) {
        std::shared_ptr<std::string> s_ptr;
        if (packet->getType() != PacketType::TYPE_NULL) {
            s_ptr = std::make_shared<std::string>(packet->getValue());
        } else {
            s_ptr = nullptr;
        }
        result.setValue(s_ptr);
    }

    static void doIntegerPtrCallback(const PacketPtr &resp, const ResultIntegerPtrCallback &callback) {
        TairResult<std::shared_ptr<int64_t>> result;
        IntegerPacket *p = resp.get()->packet_cast<IntegerPacket>();
        if (p) {
            result.setValue(std::make_shared<int64_t>(p->getValue()));
        } else {
            result.setValue(nullptr);
        }
        callback(result);
    }

    static void scanResultBuilder(ArrayPacket *ap, TairResult<ScanResult> &result) {
        ScanResult sr;
        auto packet_array = ap->getPacketArray();
        runtimeAssert(packet_array.size() == 2);
        auto *cursor_ptr = packet_array[0]->packet_cast<BulkStringPacket>();
        auto *results_ptr = packet_array[1]->packet_cast<ArrayPacket>();
        if (!cursor_ptr || !results_ptr) {
            result.setErr("FATAL: decode scan result failed.");
            return;
        }
        sr.cursor = cursor_ptr->moveBulkStr();
        results_ptr->moveBulks(sr.results);
        result.setValue(sr);
    }

    static void geoposResultBuilder(ArrayPacket *ap, TairResult<GeoPosResult> &result) {
        GeoPosResult gr;
        auto packet_array = ap->getPacketArray();
        for (auto &pos : packet_array) {
            ArrayPacket *posPacket = pos->packet_cast<ArrayPacket>();
            if (posPacket && posPacket->getType() != PacketType::TYPE_NULL) {
                auto *longitude = posPacket->getPacketArray()[0]->packet_cast<BulkStringPacket>();
                auto *latitude = posPacket->getPacketArray()[1]->packet_cast<BulkStringPacket>();
                if (!longitude || !latitude) {
                    result.setErr("FATAL: decode geopos result failed.");
                    return;
                }
                gr.push_back(std::make_optional(std::make_pair(longitude->moveBulkStr(), latitude->moveBulkStr())));
            } else {
                gr.push_back(std::nullopt);
            }
        }
        result.setValue(gr);
    }

    static void xpendingResultBuilder(ArrayPacket *ap, TairResult<XPendingResult> &result) {
        XPendingResult xpr;
        auto packet_array = ap->getPacketArray();
        runtimeAssert(packet_array.size() == 4);
        xpr.pel_number = packet_array[0]->packet_cast<IntegerPacket>()->getValue();
        if (xpr.pel_number == 0) {
            result.setValue(xpr);
            return;
        }
        xpr.startid = packet_array[1]->packet_cast<BulkStringPacket>()->moveBulkStr();
        xpr.endid = packet_array[2]->packet_cast<BulkStringPacket>()->moveBulkStr();
        for (auto &message : packet_array[3]->packet_cast<ArrayPacket>()->getPacketArray()) {
            ArrayPacket *map = message->packet_cast<ArrayPacket>();
            auto name = map->getPacketArray()[0]->packet_cast<BulkStringPacket>()->moveBulkStr();
            auto count = map->getPacketArray()[1]->packet_cast<BulkStringPacket>()->moveBulkStr();
            xpr.messages.emplace_back(name, count);
        }
        result.setValue(xpr);
    }

    static void xrangeResultBuilder(ArrayPacket *ap, TairResult<std::vector<XRangeResult>> &result) {
        std::vector<XRangeResult> results;
        auto packet_array = ap->getPacketArray();
        for (auto &pos : packet_array) {
            ArrayPacket *posPacket = pos->packet_cast<ArrayPacket>();
            XRangeResult xr;
            auto *id_ptr = posPacket->getPacketArray()[0]->packet_cast<BulkStringPacket>();
            auto *values_ptr = posPacket->getPacketArray()[1]->packet_cast<ArrayPacket>();
            if (!id_ptr || !values_ptr) {
                result.setErr("FATAL: decode xrange result failed.");
                return;
            }
            xr.id = id_ptr->moveBulkStr();
            values_ptr->moveBulks(xr.values);
            results.push_back(xr);
        }
        result.setValue(results);
    }

    static void xreadResultBuilder(ArrayPacket *ap, TairResult<std::vector<XReadResult>> &result) {
        std::vector<XReadResult> results;
        if (ap->getType() == PacketType::TYPE_NULL) {
            result.setValue(results);
            return;
        }
        auto packet_array = ap->getPacketArray();
        for (auto &pos : packet_array) {
            ArrayPacket *posPacket = pos->packet_cast<ArrayPacket>();
            XReadResult xr;
            xr.streamname = posPacket->getPacketArray()[0]->packet_cast<BulkStringPacket>()->moveBulkStr();
            auto sinfo = posPacket->getPacketArray()[1]->packet_cast<ArrayPacket>()->getPacketArray();
            if (!sinfo.empty()) {
                StreamInfo si;
                si.id = sinfo[0]->packet_cast<ArrayPacket>()->getPacketArray()[0]->packet_cast<BulkStringPacket>()->moveBulkStr();
                sinfo[0]->packet_cast<ArrayPacket>()->getPacketArray()[1]->packet_cast<ArrayPacket>()->moveBulks(si.values);
                xr.infos.push_back(si);
            }
            results.push_back(xr);
        }
        result.setValue(results);
    }

    template <typename PACKET_TYPE, typename VALUE, typename B, typename C>
    static void doCallbackByBuilder(const PacketPtr &resp, const B &builder, const C &callback) {
        TairResult<VALUE> result;
        if (resp) {
            PACKET_TYPE *p = resp.get()->packet_cast<PACKET_TYPE>();
            if (p != nullptr) {
                builder(p, result);
            } else {
                std::string error;
                if (RESPPacketHelper::getReplyError(resp.get(), error)) {
                    result.setErr(error);
                } else {
                    result.setErr("unknown return type");
                }
            }
        } else {
            result.setErr("connection error, response is null");
        }
        callback(result);
    }

    template <typename RESULT>
    static bool waitFuture(const char *func_name, std::future<TairResult<RESULT>> &future, RESULT &value, std::string &err, int64_t timeout_ms) {
        auto status = future.wait_for(std::chrono::milliseconds(timeout_ms));
        if (status != std::future_status::ready) {
            LOG_ERROR("FATAL: tair client call {} timeout", func_name);
            return false;
        }
        auto result = future.get();
        if (!result.isSuccess()) {
            err = result.getErr();
            LOG_ERROR("tair client call {} failed: {}", func_name, err);
            return false;
        }
        value = result.getValue();
        return true;
    }
};

} // namespace tair::client
