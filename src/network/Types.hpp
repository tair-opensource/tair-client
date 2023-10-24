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
#include <memory>

namespace tair::network {

using socket_t = int;

class Timer;
class Buffer;
class Connector;
class TcpConnection;
class TcpClient;
class TcpServer;
class EventLoop;
class DnsResolver;
class SignalEventWatcher;
class EventLoopThread;
class EventLoopThreadPool;

using TimerId = uint64_t;
using TimerHandler = std::function<void(EventLoop *)>;
using Callback = std::function<void()>;
using ChooseLoopCallback = std::function<bool(EventLoop *)>;
using LoopTaskCallback = std::function<void(EventLoop *)>;
using ExpectedLoopCallback = std::function<EventLoop *()>;
using AllLoopTaskCallback = std::function<void(std::vector<EventLoop *>, size_t, size_t)>;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TcpClientPtr = std::shared_ptr<TcpClient>;
using ConnectorPtr = std::shared_ptr<Connector>;
using DnsResolverPtr = std::shared_ptr<DnsResolver>;

using NewConnectionCallback = std::function<void(socket_t, const std::string &, const std::string &, bool is_tls)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using BeforeReadEventCallback = std::function<bool(const TcpConnectionPtr &)>;
using BeforeWriteEventCallback = std::function<bool(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *)>;
using EventLoopInitCallback = std::function<void(EventLoop *, int)>;
using EventLoopDestroyCallback = std::function<void(EventLoop *)>;
using EventLoopBeforeSleepCallBack = std::function<void(EventLoop *)>;
using EventLoopAfterSleepCallBack = std::function<void(EventLoop *)>;
using EventLoopThreadExitCheckCallBack = std::function<bool(size_t, EventLoop *)>;

} // namespace tair::network
