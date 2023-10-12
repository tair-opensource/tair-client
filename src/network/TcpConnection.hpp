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

#include <any>
#include <atomic>
#include <future>
#include <string>

#include "common/Noncopyable.hpp"
#include "network/Buffer.hpp"
#include "network/Types.hpp"

namespace tair::network {

using common::Noncopyable;

class EventLoop;
class Channel;

class TcpConnection : private Noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    enum Status {
        kDisconnected = 0,
        kConnecting = 1,
        kConnected = 2,
        kDisconnecting = 3,
    };

public:
    TcpConnection(socket_t sockfd, const std::string &local_ip_port, const std::string &remote_ip_port);
    virtual ~TcpConnection();

    void close();

    void send(const std::string &str) {
        send(str.data(), str.size());
    }

    void send(const Buffer &buf) {
        send(buf.data(), buf.size());
    }

    void send(const char *data) {
        send(data, strlen(data));
    }

    void send(const void *data, size_t len) {
        send(std::string_view((char *)data, len));
    }

    void send(std::string &&str);
    void send(Buffer &&buf);

    void send(const std::string_view &str);

    // in order to support the merge resp in one write
    void sendOutputBuffer();

    void setLoop(EventLoop *loop) {
        loop_ = loop;
    }

    EventLoop *loop() const {
        return loop_;
    }

    socket_t sockfd() const {
        return fd_;
    }

    const std::string &getRemoteIpPort() const {
        return remote_ip_port_;
    }

    int getRemotePort() const {
        return remote_port_;
    }

    const std::string &getLocalIpPort() const {
        return local_ip_port_;
    }

    int getLocalPort() const {
        return local_port_;
    }

    uint32_t getVpcTunnelID() const {
        return vpc_tunnel_id_;
    }

    const std::string &getVirtualIp() const {
        return virtual_ip_;
    }

    const int &getVirtualPort() const {
        return virtual_port_;
    }

    virtual bool isTLSConnection() const {
        return false;
    }

    bool isConnected() const {
        return status_ == kConnected;
    }

    bool isConnecting() const {
        return status_ == kConnecting;
    }

    bool isDisconnected() const {
        return status_ == kDisconnected;
    }

    bool isDisconnecting() const {
        return status_ == kDisconnecting;
    }

    const Buffer &getInputBuffer() const {
        return input_buffer_;
    }

    const Buffer &getOutputBuffer() const {
        return output_buffer_;
    }

    Buffer &getOutputBufferForWrite() {
        return output_buffer_;
    }

    Status status() const {
        return status_;
    }

    void reserveInputBuffer(size_t len) {
        input_buffer_.reserve(len);
    }

    void reserveOutputBuffer(size_t len) {
        output_buffer_.reserve(len);
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

    void setAfterReadEventCallback(const AfterReadEventCallback &callback) {
        after_read_event_callback_ = callback;
    }

    void setAfterWriteEventCallback(const AfterWriteEventCallback &callback) {
        after_write_event_callback_ = callback;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &callback) {
        write_complete_callback_ = callback;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback &callback, size_t mark) {
        high_water_mark_callback_ = callback;
        high_water_mark_ = mark;
    }

    // inner callback, not for user
    void setCloseCallback(const CloseCallback &callback) {
        close_callback_ = callback;
    }

    // strange interface, in order to support the processing of pending requests under special asynchronous logic
    // MUST ensure isConnected() before call this
    void doMessageCallback() {
        if (message_callback_) {
            message_callback_(shared_from_this(), &input_buffer_);
        }
    }

    void setContext(const std::any &context) {
        context_ = context;
    }

    const std::any &getContext() const {
        return context_;
    }

    virtual void attachedToLoop(EventLoop *loop);

    void moveToNewLoop(EventLoop *new_loop, const Callback &success_cb, const Callback &fail_cb);

    bool hasReadableEvent() const;
    bool hasWritableEvent() const;
    void enableReadEvent();
    void disableReadEvent();
    void enableWriteEvent();
    void disableWriteEvent();

    std::string statusToString() const;

protected:
    virtual void handleRead();
    virtual void handleWrite();
    virtual void handleClose();
    virtual void handleError();

    void sendInLoop(const std::string &str) {
        sendInLoop(str.data(), str.size());
    }

    void sendInLoop(const Buffer &buf) {
        sendInLoop(buf.data(), buf.size());
    }

    void sendInLoop(const std::string_view &str) {
        sendInLoop(str.data(), str.size());
    }

    virtual void sendInLoop(const void *data, size_t len);

    void moveToNewLoopInLoop(EventLoop *new_loop, const Callback &success_cb, const Callback &fail_cb);
    void detachFromLoopAndReset();
    void attachToNewLoop(EventLoop *new_loop);

protected:
    EventLoop *loop_;
    int fd_;

    std::string local_ip_port_;
    int local_port_ = -1;

    std::string remote_ip_port_;
    int remote_port_ = -1;

    uint32_t vpc_tunnel_id_ = 0;
    std::string virtual_ip_;
    int virtual_port_ = -1;

    std::atomic<Status> status_;

    std::shared_ptr<Channel> channel_;
    Buffer input_buffer_;
    Buffer output_buffer_;

    size_t high_water_mark_ = 128 * 1024 * 1024; // Default 128MB

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    BeforeReadEventCallback before_read_event_callback_;
    BeforeWriteEventCallback before_write_event_callback_;
    AfterReadEventCallback after_read_event_callback_;
    AfterWriteEventCallback after_write_event_callback_;
    WriteCompleteCallback write_complete_callback_;
    HighWaterMarkCallback high_water_mark_callback_;
    CloseCallback close_callback_;

    std::any context_;
};

} // namespace tair::network
