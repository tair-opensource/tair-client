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
#include "network/TcpServer.hpp"

#include "common/Logger.hpp"
#include "network/Acceptor.hpp"
#include "network/EventLoop.hpp"
#include "network/EventLoopThreadPool.hpp"
#include "network/Sockets.hpp"
#include "network/TcpConnection.hpp"
#include "network/TlsConnection.hpp"

namespace tair::network {

TcpServer::TcpServer(EventLoop *loop, const std::string &listen_ip_port, size_t thread_num, std::string name)
    : stopped_(true), loop_(loop), name_(name), conn_count_(0) {
    runtimeAssert(listen_ip_port.starts_with("tcp://") || listen_ip_port.starts_with("tls://"));
    listen_ip_ports_.emplace(listen_ip_port);
    loop_thread_pool_ = std::make_unique<EventLoopThreadPool>(loop, thread_num, name);
}

TcpServer::TcpServer(EventLoop *loop, const std::set<std::string> &listen_ip_ports, size_t thread_num, std::string name)
    : stopped_(true), loop_(loop), name_(name), listen_ip_ports_(listen_ip_ports), conn_count_(0) {
    for (auto &ip_port : listen_ip_ports_) {
        runtimeAssert(ip_port.starts_with("tcp://") || ip_port.starts_with("tls://"));
    }
    loop_thread_pool_ = std::make_unique<EventLoopThreadPool>(loop, thread_num, name);
}

TcpServer::~TcpServer() {
    runtimeAssert(connections_.empty());
    runtimeAssert(acceptors_.empty());
}

AcceptorPtr TcpServer::createAcceptor(const std::string &ip_port) {
    auto acceptor = std::make_shared<Acceptor>(loop_, ip_port);
    auto callback = [this](socket_t sockfd, const std::string &local_ip_port, const std::string &remote_ip_port, bool is_tls) {
        handleNewConnection(sockfd, local_ip_port, remote_ip_port, is_tls);
    };
    acceptor->setNewConnectionCallback(callback);
    return acceptor;
}

bool TcpServer::start() {
    LockGuard lock(mutex_);
    if (stopped_) {
        LOG_DEBUG("start TcpServer");
        stopped_ = false;
        loop_thread_pool_->setLoopInitCallback(init_callback_);
        loop_thread_pool_->setLoopDestroyCallback(destroy_callback_);
        loop_thread_pool_->setBeforeSleepCallBack(before_sleep_call_back_);
        loop_thread_pool_->setAfterSleepCallBack(after_sleep_call_back_);
        loop_thread_pool_->setAfterResizeThreadExitCheckCallBack(thread_exit_check_callback_);
        loop_thread_pool_->start();
        runtimeAssert(loop_thread_pool_->isRunning());
        for (auto &ip_port : listen_ip_ports_) {
            auto acceptor = createAcceptor(ip_port);
            if (!acceptor->listen()) {
                return false;
            }
            acceptor->startAccept();
            real_listen_ip_ports_.emplace(acceptor->getRealListenIpPort());
            acceptors_.emplace_back(std::move(acceptor));
        }
        runtimeAssert(acceptors_.size() == listen_ip_ports_.size());
        runtimeAssert(acceptors_.size() == real_listen_ip_ports_.size());
        return true;
    }
    return false;
}

void TcpServer::stop() {
    LockGuard lock(mutex_);
    if (!stopped_) {
        LOG_DEBUG("stop TcpServer");
        if (loop_->isInLoopThread()) {
            stopInLoop();
        } else {
            loop_->queueInLoop([this](EventLoop *) {
                LockGuard lock(mutex_);
                stopInLoop();
            });
        }
    }
}

bool TcpServer::resizeIOThreadPoolSize(size_t new_thread_num) {
    return loop_thread_pool_->resizeThreadPoolSize(new_thread_num);
}

size_t TcpServer::ioThreadNum() const {
    return loop_thread_pool_->ioThreadNum();
}

size_t TcpServer::availableIOThreadNum() const {
    return loop_thread_pool_->availableIOThreadNum();
}

size_t TcpServer::runInAllLoop(const LoopTaskCallback &callback) {
    if (unlikely(!loop_thread_pool_)) {
        return 0;
    }
    return loop_thread_pool_->runInAllLoop(callback);
}

size_t TcpServer::queueInAllLoop(const LoopTaskCallback &callback) {
    if (unlikely(!loop_thread_pool_)) {
        return 0;
    }
    return loop_thread_pool_->queueInAllLoop(callback);
}

size_t TcpServer::runInRandomLoop(const LoopTaskCallback &callback) {
    if (unlikely(!loop_thread_pool_)) {
        return 0;
    }
    return loop_thread_pool_->runInRandomLoop(callback);
}

size_t TcpServer::runInChoosedLoop(const ChooseLoopCallback &choose_callback, const LoopTaskCallback &callback) {
    if (unlikely(!loop_thread_pool_)) {
        return 0;
    }
    return loop_thread_pool_->runInChoosedLoop(choose_callback, callback);
}

void TcpServer::runWithAllLoop(const AllLoopTaskCallback &callback) {
    if (unlikely(!loop_thread_pool_)) {
        return;
    }
    loop_thread_pool_->runWithAllLoop(callback);
}

bool TcpServer::addListenIpPort(const std::string &ip_port) {
    LockGuard lock(mutex_);
    if (listen_ip_ports_.contains(ip_port)) {
        return false;
    }
    if (!stopped_) {
        auto acceptor = createAcceptor(ip_port);
        if (!acceptor->listen()) {
            return false;
        }
        acceptor->startAccept();
        real_listen_ip_ports_.emplace(acceptor->getRealListenIpPort());
        acceptors_.emplace_back(std::move(acceptor));
    }
    listen_ip_ports_.emplace(ip_port);
    return true;
}

bool TcpServer::removeListenIpPort(const std::string &ip_port) {
    LockGuard lock(mutex_);
    if (!listen_ip_ports_.contains(ip_port)) {
        return false;
    }
    if (!stopped_) {
        auto iter = acceptors_.begin();
        while (iter != acceptors_.end()) {
            if ((*iter)->getOriListenIpPort() == ip_port) {
                (*iter)->stop();
                real_listen_ip_ports_.erase((*iter)->getRealListenIpPort());
                acceptors_.erase(iter);
            } else {
                iter++;
            }
        }
    }
    listen_ip_ports_.erase(ip_port);
    return false;
}

void TcpServer::stopThreadPool() {
    LOG_TRACE("stop EventLoopThreadPool");
    runtimeAssert(loop_->isInLoopThread());
    runtimeAssert(loop_thread_pool_);
    loop_thread_pool_->stop();
    loop_thread_pool_->join();
    runtimeAssert(!loop_thread_pool_->isRunning());
    loop_thread_pool_.reset();
    if (closed_callback_) {
        closed_callback_();
    }
}

void TcpServer::stopInLoop() {
    LOG_TRACE("stop TcpServer in loop now");
    runtimeAssert(loop_->isInLoopThread());
    if (stopped_) {
        return;
    }
    stopped_ = true;
    for (auto &acceptor : acceptors_) {
        acceptor->stop();
        acceptor.reset();
    }
    acceptors_.clear();
    if (connections_.empty()) {
        stopThreadPool();
    } else {
        LOG_TRACE("close connections");
        // close_callback_ in conn will call connections_.erase(), so copy it
        auto connections = connections_;
        for (auto &pair : connections) {
            auto fd = pair.first;
            auto &conn = pair.second;
            runtimeAssert(fd = conn->sockfd());
            if (conn->isConnected()) {
                LOG_TRACE("close TcpConnection, fd={}", conn->sockfd());
                // TODO FIXME: maybe conn->loop_ is nullptr
                conn->close();
            } else {
                LOG_TRACE("Do not need close this TcpConnection, fd={} status={}", conn->sockfd(), conn->statusToString());
            }
        }
        // stopThreadPool in last removeConnection
    }
    LOG_DEBUG("TcpServer exited");
}

void TcpServer::handleNewConnection(socket_t sockfd, const std::string &local_ip_port, const std::string &remote_ip_port, bool is_tls) {
    LOG_TRACE("TcpServer get new connection, fd={}", sockfd);
    runtimeAssert(loop_->isInLoopThread());
    if (stopped_) {
        LOG_WARN("this={} TcpServer is at stopped_ status, discard this socket fd={} remote_addr={}",
                 (void *)this, sockfd, remote_ip_port);
        sockets::closeSocket(sockfd);
        return;
    }
    if (keepalive_seconds_ > 0) {
        sockets::setKeepAlive(sockfd, true, keepalive_seconds_);
    }
    TcpConnectionPtr conn = nullptr;
    // chose TcpConnection or TlsConnection.
    if (!is_tls) {
        conn = std::make_shared<TcpConnection>(sockfd, local_ip_port, remote_ip_port);
    } else {
        conn = std::make_shared<TlsConnection>(sockfd, local_ip_port, remote_ip_port, TlsConnection::kServer);
    }
    conn->setConnectionCallback(connection_callback_);
    conn->setMessageCallback(message_callback_);
    conn->setWriteCompleteCallback(write_complete_callback_);
    conn->setBeforeReadEventCallback(before_read_event_callback_);
    conn->setBeforeWriteEventCallback(before_write_event_callback_);
    conn->setHighWaterMarkCallback(high_water_mark_callback_, high_water_mark_);
    conn->setCloseCallback([this](const TcpConnectionPtr &conn) {
        removeConnection(conn);
    });

    runtimeAssert(connections_.find(conn->sockfd()) == connections_.end());
    connections_[conn->sockfd()] = conn;
    conn_count_++;
    runtimeAssert(conn_count_ == connections_.size());

    auto callback = [conn](EventLoop *loop) {
        conn->attachedToLoop(loop);
    };

    // attach conn to chose loop
    if (policy_ == kFdHashing) {
        loop_thread_pool_->runInLoopByHash(sockfd, callback);
    } else {
        runtimeAssert(policy_ == kRoundRobin);
        loop_thread_pool_->runInNextlLoop(callback);
    }
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    // remove connection in listening EventLoop
    if (loop_->isInLoopThread()) {
        removeConnectionInLoop(conn);
    } else {
        loop_->queueInLoop([this, conn](EventLoop *) {
            removeConnectionInLoop(conn);
        });
    }
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    runtimeAssert(loop_->isInLoopThread());
    runtimeAssert(connections_.find(conn->sockfd()) != connections_.end());
    connections_.erase(conn->sockfd());
    conn_count_--;
    runtimeAssert(conn_count_ == connections_.size());
    LOG_TRACE("removed Connection in loop, conn={} fd={}, after connections_.size()={}",
              (void *)conn.get(), conn->sockfd(), connections_.size());
    if (stopped_ && connections_.empty()) {
        // At last, we stop all the working threads
        stopThreadPool();
    }
}

} // namespace tair::network
