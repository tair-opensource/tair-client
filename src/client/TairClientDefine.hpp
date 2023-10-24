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

#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "protocol/codec/CodecFactory.hpp"
#include "client/TairResult.hpp"
#include "client/results/ResultsAll.hpp"

namespace tair::client {

using protocol::PacketPtr;

using CommandArgv = std::vector<std::string>;

template <typename T>
using Function = std::function<T>;

template <typename T>
using InitializerList = std::initializer_list<T>;

using ResultStringCallback = Function<void(const TairResult<std::string> &)>;
using ResultStringPtrCallback = Function<void(const TairResult<std::shared_ptr<std::string>> &)>;
using ResultVectorStringCallback = Function<void(const TairResult<std::vector<std::string>> &)>;
using ResultVectorStringPtrCallback = Function<void(const TairResult<std::vector<std::shared_ptr<std::string>>> &)>;
using ResultIntegerCallback = Function<void(const TairResult<int64_t> &)>;
using ResultIntegerPtrCallback = Function<void(const TairResult<std::shared_ptr<int64_t>> &)>;
using ResultVectorIntegerCallback = Function<void(const TairResult<std::vector<int64_t>> &)>;
using ResultVectorIntegerPtrCallback = Function<void(const TairResult<std::vector<std::shared_ptr<int64_t>>> &)>;
using ResultScanCallback = Function<void(const TairResult<ScanResult> &)>;
using GeoPosResult = std::vector<std::optional<std::pair<std::string, std::string>>>;
using ResultGeoposCallback = Function<void(const TairResult<GeoPosResult> &)>;
using ResultXPendingCallback = Function<void(const TairResult<XPendingResult> &)>;
using ResultXRangeCallback = Function<void(const TairResult<std::vector<XRangeResult>> &)>;
using ResultXreadCallback = Function<void(const TairResult<std::vector<XReadResult>> &)>;

class ITairClient;
using ResultPacketCallback = std::function<void(ITairClient *client, const PacketPtr &req, const PacketPtr &resp)>;
using ResultPacketAndLatencyCallback = std::function<void(ITairClient *client, const PacketPtr &req, const PacketPtr &resp, int64_t latency_us)>;

} // namespace tair::client
