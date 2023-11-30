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

#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <vector>

#include "common/ClockTime.hpp"
#include "common/CountDownLatch.hpp"
#include "common/Noncopyable.hpp"
#include "network/Duration.hpp"
#include "network/TcpConnection.hpp"
#include "network/Types.hpp"
#include "protocol/codec/CodecFactory.hpp"
#include "client/TairClientDefine.hpp"
#include "client/TairResult.hpp"

namespace tair::client {

using common::Noncopyable;
using common::CountDownLatch;
using common::ClockTime;
using common::LockGuard;
using common::Mutex;
using network::Duration;
using network::Buffer;
using network::TcpClient;
using network::TcpClientPtr;
using network::TcpConnectionPtr;
using network::EventLoop;
using network::EventLoopThread;
using network::ConnectionCallback;
using protocol::CodecPtr;
using protocol::Packet;
using protocol::PacketUniqPtr;

using RespPacketPtrCallback = std::function<void(const PacketPtr &req, const PacketPtr &resp, int64_t latency_us)>;

class TairBaseClient : private Noncopyable {
public:
    TairBaseClient();
    explicit TairBaseClient(EventLoop *loop);
    virtual ~TairBaseClient();

    void setServerAddr(const std::string &addr);
    const std::string &getServerAddr() const;

    void setUser(const std::string &user);
    void setPassword(const std::string &password);
    void setConnectingTimeoutMs(int timeout_ms);
    void setReconnectIntervalMs(int timeout);
    void setAutoReconnect(bool reconnect);
    void setKeepAliveSeconds(int seconds);

    std::future<TairResult<std::string>> connect() EXCLUDES(mutex_);
    void disconnect();
    void reconnect();
    bool isConnected() const;

    void auth(const std::string &password, const ResultStringCallback &callback);
    void auth(const std::string &user, const std::string &password, const ResultStringCallback &callback);
    void clientSetName(const std::string &name, const ResultStringCallback &callback);

protected:
    void sendCommandInLoop(const PacketPtr &req, const RespPacketPtrCallback &callback);
    void sendCommand(const PacketPtr &req, const RespPacketPtrCallback &callback);
    void sendCommand(CommandArgv &&argv, const RespPacketPtrCallback &callback);
    void sendCommand(const CommandArgv &argv, const RespPacketPtrCallback &callback);

    virtual void onConnected();
    virtual void onDisconnected() EXCLUDES(mutex_);
    virtual void onRecvResponse(const PacketPtr &resp);

    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf);
    void clearCallbacks();

private:
    bool doConnect();
    void authentication();
    void initEventLoop();
    void clientCron();
    void assertNotInCallbackContext();

protected:
    // User parameter
    std::string server_addr_;
    std::string user_;
    std::string password_;
    int connecting_timeout_ms_ = 2000;
    int reconnect_interval_ms_ = -1;
    bool auto_reconnect_ = true;
    int keepalive_seconds_ = 60;

    // Tcp client resource
    CodecPtr codec_;
    TcpClientPtr tcp_client_;
    int64_t reconnect_timer_id_ = -1;
    int64_t last_send_req_time_ms_ = 0;
    int64_t last_recv_resp_time_ms_ = 0;
    std::unique_ptr<EventLoopThread> loop_thread_;
    EventLoop *loop_ = nullptr;

    struct CallBackContext {
        CallBackContext(const PacketPtr &r, const RespPacketPtrCallback &cb)
            : req(r), callback(cb) {}
        int64_t init_time = ClockTime::intervalUs();
        PacketPtr req;
        RespPacketPtrCallback callback;
    };
    bool in_callback_context_ = false;
    std::deque<CallBackContext> callbacks_;

    Mutex mutex_;
    std::unique_ptr<std::promise<TairResult<std::string>>> auth_promise_ GUARDED_BY(mutex_);
};

} // namespace tair::client
