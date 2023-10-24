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
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"
#include "network/Types.hpp"

namespace tair::network {

using common::Mutex;
using common::LockGuard;
using common::Noncopyable;

class EventLoop;
class EventLoopThreadPool;
class Acceptor;

using AcceptorPtr = std::shared_ptr<Acceptor>;

class TcpServer final : Noncopyable {
public:
    enum Policy {
        kRoundRobin = 1,
        kFdHashing = 2,
    };

public:
    TcpServer(EventLoop *loop, const std::string &listen_ip_port_, size_t thread_num, std::string name);
    TcpServer(EventLoop *loop, const std::set<std::string> &listen_ip_ports, size_t thread_num, std::string name);
    ~TcpServer();

    bool start() EXCLUDES(mutex_);
    void stop();

    bool resizeIOThreadPoolSize(size_t new_thread_num);

    size_t ioThreadNum() const;
    size_t availableIOThreadNum() const;

    size_t runInAllLoop(const LoopTaskCallback &callback);
    size_t queueInAllLoop(const LoopTaskCallback &callback);
    size_t runInRandomLoop(const LoopTaskCallback &callback);
    size_t runInChoosedLoop(const ChooseLoopCallback &choose_callback, const LoopTaskCallback &callback);
    void runWithAllLoop(const AllLoopTaskCallback &callback);

    bool addListenIpPort(const std::string &ip_port) EXCLUDES(mutex_);
    bool removeListenIpPort(const std::string &ip_port) EXCLUDES(mutex_);

    bool isStarted() const { return !stopped_; }

    void setKeepAlive(int seconds) {
        keepalive_seconds_ = seconds;
    }

    void setDispatchPolicy(Policy policy) {
        policy_ = policy;
    }

    void setLoopInitCallback(const EventLoopInitCallback &callback) {
        init_callback_ = callback;
    }

    void setLoopDestroyCallback(const EventLoopDestroyCallback &callback) {
        destroy_callback_ = callback;
    }

    void setBeforeSleepCallBack(const EventLoopBeforeSleepCallBack &callback) {
        before_sleep_call_back_ = callback;
    }

    void setAfterSleepCallBack(const EventLoopAfterSleepCallBack &callback) {
        after_sleep_call_back_ = callback;
    }

    void setAfterResizeThreadExitCheckCallBack(const EventLoopThreadExitCheckCallBack &callback) {
        thread_exit_check_callback_ = callback;
    }

    void setConnectionCallback(const ConnectionCallback &callback) {
        connection_callback_ = callback;
    }

    void setMessageCallback(const MessageCallback &callback) {
        message_callback_ = callback;
    }

    void setBeforeReadEventCallback(const BeforeReadEventCallback &callback) {
        before_read_event_callback_ = callback;
    }

    void setBeforeWriteEventCallback(const BeforeWriteEventCallback &callback) {
        before_write_event_callback_ = callback;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &callback) {
        write_complete_callback_ = callback;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback &callback, size_t mark) {
        high_water_mark_callback_ = callback;
        high_water_mark_ = mark;
    }

    void setClosedCallback(Callback callback) {
        closed_callback_ = callback;
    }

    std::set<std::string> getRealListenIpPorts() const {
        LockGuard lock(mutex_);
        return real_listen_ip_ports_;
    }

    uint64_t getConnCount() const {
        return conn_count_;
    }

private:
    AcceptorPtr createAcceptor(const std::string &ip_port);
    void stopThreadPool();
    void stopInLoop() REQUIRES(mutex_);
    void handleNewConnection(socket_t sockfd, const std::string &local_ip_port, const std::string &remote_ip_port, bool is_tls);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

private:
    std::atomic<bool> stopped_;
    EventLoop *loop_;
    std::string name_;
    int keepalive_seconds_ = 0;

    Policy policy_ = kRoundRobin;
    std::unique_ptr<EventLoopThreadPool> loop_thread_pool_;

    mutable Mutex mutex_;
    std::set<std::string> listen_ip_ports_ GUARDED_BY(mutex_);
    std::set<std::string> real_listen_ip_ports_ GUARDED_BY(mutex_);
    std::vector<AcceptorPtr> acceptors_ GUARDED_BY(mutex_);

    size_t high_water_mark_ = 128 * 1024 * 1024; // Default 128MB

    EventLoopInitCallback init_callback_;
    EventLoopDestroyCallback destroy_callback_;
    EventLoopBeforeSleepCallBack before_sleep_call_back_;
    EventLoopAfterSleepCallBack after_sleep_call_back_;
    EventLoopThreadExitCheckCallBack thread_exit_check_callback_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    BeforeReadEventCallback before_read_event_callback_;
    BeforeWriteEventCallback before_write_event_callback_;
    WriteCompleteCallback write_complete_callback_;
    HighWaterMarkCallback high_water_mark_callback_;
    Callback closed_callback_;

    std::atomic<uint64_t> conn_count_;
    std::unordered_map<socket_t, TcpConnectionPtr> connections_;
};

} // namespace tair::network
