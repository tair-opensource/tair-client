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
#include "client/TairBaseClient.hpp"

#include "common/ClockTime.hpp"
#include "common/Logger.hpp"
#include "network/EventLoopThread.hpp"
#include "network/TcpClient.hpp"
#include "network/TcpConnection.hpp"
#include "protocol/packet/resp/ArrayPacket.hpp"
#include "client/TairClientInfo.hpp"
#include "client/TairResultHelper.hpp"

namespace tair::client {

using protocol::ArrayPacket;
using protocol::CodecFactory;
using protocol::CodecType;
using protocol::DState;
using protocol::IntegerPacket;
using protocol::SimpleStringPacket;
using common::ClockTime;

TairBaseClient::TairBaseClient() {
    initEventLoop();
}

TairBaseClient::TairBaseClient(EventLoop *loop) {
    loop_ = loop;
    if (!loop_) {
        initEventLoop();
    }
}

TairBaseClient::~TairBaseClient() {
    disconnect();
    if (loop_thread_) {
        loop_thread_->stop();
        loop_thread_->join();
        loop_thread_.reset();
    }
}

void TairBaseClient::initEventLoop() {
    loop_thread_ = std::make_unique<EventLoopThread>("client-io");
    loop_thread_->start();
    loop_ = loop_thread_->loop();
}

void TairBaseClient::setServerAddr(const std::string &addr) {
    server_addr_ = addr;
}

const std::string &TairBaseClient::getServerAddr() const {
    return server_addr_;
}

void TairBaseClient::setUser(const std::string &user) {
    user_ = user;
}

void TairBaseClient::setPassword(const std::string &password) {
    password_ = password;
}

void TairBaseClient::setConnectingTimeoutMs(int timeout_ms) {
    connecting_timeout_ms_ = timeout_ms;
}

void TairBaseClient::setReconnectIntervalMs(int timeout_ms) {
    reconnect_interval_ms_ = timeout_ms;
}

void TairBaseClient::setAutoReconnect(bool reconnect) {
    auto_reconnect_ = reconnect;
}

void TairBaseClient::setKeepAliveSeconds(int seconds) {
    keepalive_seconds_ = seconds;
}

bool TairBaseClient::isConnected() const {
    return tcp_client_ && tcp_client_->isConnected();
}

bool TairBaseClient::doConnect() {
    assertNotInCallbackContext();
    if (server_addr_.empty() || !loop_) {
        return false;
    }
    tcp_client_ = TcpClient::create(loop_, server_addr_);
    tcp_client_->setConnectingTimeout(Duration(connecting_timeout_ms_ * Duration::kMillisecond));
    tcp_client_->setKeepAlive(keepalive_seconds_);
    tcp_client_->setConnectionCallback([this](const TcpConnectionPtr &conn) {
        onConnection(conn);
    });
    tcp_client_->setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf) {
        onMessage(conn, buf);
    });
    tcp_client_->setAutoReConnect(auto_reconnect_);
    tcp_client_->connect();
    return true;
}

std::future<TairResult<std::string>> TairBaseClient::connect() {
    LockGuard lock(mutex_);
    auth_promise_ = std::make_unique<std::promise<TairResult<std::string>>>();
    if (!doConnect()) {
        auth_promise_->set_value(TairResult<std::string>::createErr("server addr is empty or loop init failed"));
        return auth_promise_->get_future();
    }
    return auth_promise_->get_future();
}

void TairBaseClient::disconnect() {
    if (tcp_client_) {
        CountDownLatch latch;
        loop_->runInLoop([&](EventLoop *) {
            tcp_client_->disconnect();
            tcp_client_.reset();
            latch.countDown();
        });
        latch.wait();
    }
}

void TairBaseClient::reconnect() {
    LOG_INFO("Reconnect TairClient now");
    doConnect();
}

void TairBaseClient::authentication() {
    if (!user_.empty() && !password_.empty()) {
        auth(user_, password_, [this](const auto &result) {
            LockGuard lock(mutex_);
            LOG_INFO("TairClient auth user password, result: {}", result.getValue());
            if (auth_promise_) {
                auth_promise_->set_value(result);
                auth_promise_.reset();
            }
        });
    } else if (!password_.empty()) {
        auth(password_, [this](const auto &result) {
            LockGuard lock(mutex_);
            LOG_INFO("TairClient auth password, result: {}", result.getValue());
            if (auth_promise_) {
                auth_promise_->set_value(result);
                auth_promise_.reset();
            }
        });
    } else {
        LOG_DEBUG("TairClient user and password are not set, skip auth.");
        LockGuard lock(mutex_);
        if (auth_promise_) {
            auth_promise_->set_value(TairResult<std::string>::create("ok"));
            auth_promise_.reset();
        }
    }
}

void TairBaseClient::assertNotInCallbackContext() {
    if (in_callback_context_) {
        LOG_ERROR("Cannot call client api in callback context");
        runtimeAssert(!in_callback_context_);
    }
}

void TairBaseClient::clientCron() {
    runtimeAssert(reconnect_timer_id_ > 0);
    int64_t now = ClockTime::intervalMs();
    if (last_send_req_time_ms_ >= last_recv_resp_time_ms_ && now - last_send_req_time_ms_ >= reconnect_interval_ms_) {
        LOG_INFO("TairClient has not been active for a long time and will reconnect, now: {}, recv: {}, send: {}, interval: {}",
                 now, last_recv_resp_time_ms_, last_send_req_time_ms_, reconnect_interval_ms_);
        tcp_client_.reset();
        clearCallbacks();
        reconnect();
    }
}

void TairBaseClient::onConnected() {
    authentication();
    clientSetName(TairClientInfo::toJSON(), [](const auto &result) {});
    if (reconnect_interval_ms_ > 0) {
        LOG_INFO("TairClient starts a timer for black hole detection");
        auto interval = Duration(reconnect_interval_ms_ / 2 * Duration::kMillisecond);
        reconnect_timer_id_ = loop_->runEveryTimer(interval, [this](EventLoop *loop) {
            clientCron();
        });
    }
}

void TairBaseClient::onDisconnected() {
    LockGuard lock(mutex_);
    if (auth_promise_) {
        auth_promise_->set_value(TairResult<std::string>::createErr("connect to server fail, disconnected"));
        auth_promise_.reset();
    }
    if (reconnect_timer_id_ > 0) {
        LOG_INFO("TairClient stop a timer for black hole detection");
        loop_->cancelTimer(reconnect_timer_id_);
        reconnect_timer_id_ = -1;
    }
}

void TairBaseClient::onConnection(const TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
        // init codec when connect success (and reconnect success)
        codec_ = CodecFactory::getCodec(CodecType::RESP2);
        LOG_INFO("TairClient is connected: {} -> {}", conn->getLocalIpPort(), conn->getRemoteIpPort());
        onConnected();
    } else {
        LOG_INFO("TairClient is disconnected: {} -> {}", conn->getLocalIpPort(), conn->getRemoteIpPort());
        clearCallbacks();
        onDisconnected();
    }
}

void TairBaseClient::onRecvResponse(const PacketPtr &resp) {
    runtimeAssert(!callbacks_.empty());
    auto [init_time, req, callback] = callbacks_.front();
    int64_t latency_us = ClockTime::intervalUs() - init_time;
    callbacks_.pop_front();
    if (callback) {
        in_callback_context_ = true;
        callback(req, resp, latency_us);
        in_callback_context_ = false;
    }
}

void TairBaseClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf) {
    while (conn->isConnected()) {
        PacketUniqPtr packet;
        auto dstate = codec_->decodeResponse(buf, packet);
        PacketPtr resp = std::move(packet);
        if (dstate == DState::SUCCESS) {
            onRecvResponse(resp);
        } else if (dstate == DState::AGAIN) {
            if (!buf->empty()) {
                LOG_TRACE("connection not read full cmd, need read again");
            }
            break;
        } else if (dstate == DState::ERROR) {
            LOG_ERROR("redis protocol decode error: {}, connection addr: {}",
                      codec_->getErr(), conn->getRemoteIpPort());
            clearCallbacks();
            reconnect();
        } else {
            // LOG_FATAL will abort this process
            LOG_CRITICAL("codec return a unknown state: {}", (int)dstate);
        }
        if (conn->isConnected()) {
            if (reconnect_interval_ms_ > 0) {
                last_recv_resp_time_ms_ = ClockTime::intervalMs();
            }
        }
    }
}

void TairBaseClient::clearCallbacks() {
    assertNotInCallbackContext();
    while (!callbacks_.empty()) {
        auto [init_time, req, callback] = callbacks_.front();
        int64_t latency_us = ClockTime::intervalUs() - init_time;
        callbacks_.pop_front();
        if (callback) {
            in_callback_context_ = true;
            callback(req, nullptr, latency_us);
            in_callback_context_ = false;
        }
    }
}

void TairBaseClient::sendCommandInLoop(const PacketPtr &req, const RespPacketPtrCallback &callback) {
    runtimeAssert(loop_->isInLoopThread());
    if (!tcp_client_) { // disconnected
        in_callback_context_ = true;
        callback(req, nullptr, 0);
        in_callback_context_ = false;
        return;
    }
    callbacks_.emplace_back(CallBackContext(req, callback));
    Buffer buf;
    codec_->encodeRequest(&buf, req.get());
    auto conn = tcp_client_->connection();
    conn->send(buf);
    if (reconnect_interval_ms_ > 0) {
        last_send_req_time_ms_ = ClockTime::intervalMs();
    }
}

void TairBaseClient::sendCommand(const PacketPtr &req, const RespPacketPtrCallback &callback) {
    if (loop_->isInLoopThread()) {
        sendCommandInLoop(req, callback);
    } else {
        loop_->queueInLoop([this, req, callback](EventLoop *) {
            sendCommandInLoop(req, callback);
        });
    }
}

void TairBaseClient::sendCommand(CommandArgv &&argv, const RespPacketPtrCallback &callback) {
    PacketPtr req = std::make_shared<ArrayPacket>(std::move(argv));
    sendCommand(req, callback);
}

void TairBaseClient::sendCommand(const CommandArgv &argv, const RespPacketPtrCallback &callback) {
    PacketPtr req = std::make_shared<ArrayPacket>(argv);
    sendCommand(req, callback);
}

void TairBaseClient::auth(const std::string &password, const ResultStringCallback &callback) {
    sendCommand({"auth", password}, [callback](auto &, auto &resp, int64_t) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairBaseClient::auth(const std::string &user, const std::string &password, const ResultStringCallback &callback) {
    sendCommand({"auth", user, password}, [callback](auto &, auto &resp, int64_t) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairBaseClient::clientSetName(const std::string &name, const ResultStringCallback &callback) {
    sendCommand({"client", "setname", name}, [callback](auto &, auto &resp, int64_t) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

} // namespace tair::client
