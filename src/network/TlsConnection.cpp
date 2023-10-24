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
#include "network/TlsConnection.hpp"

#include "common/Logger.hpp"
#include "common/SystemUtil.hpp"
#include "network/Channel.hpp"
#include "network/EventLoop.hpp"
#include "network/NetworkStat.hpp"
#include "network/Sockets.hpp"
#include "network/TcpConnection.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>

namespace tair::network {

using common::SystemUtil;

TlsConnection::TlsConnection(socket_t sockfd, const std::string &local_ip_port,
                             const std::string &remote_ip_port, TlsConnection::Type type)
    : TcpConnection(sockfd, local_ip_port, remote_ip_port) {
    auto ssl_ctx = TlsOptions::instance().getSslContext();
    ssl_ = SSL_new(ssl_ctx.get());
    SSL_set_fd(ssl_, sockfd);
    type_ = type;
    LOG_TRACE("TlsConnection() this: {}", (void *)this);
}

// @override
void TlsConnection::attachedToLoop(EventLoop *loop) {
    runtimeAssert(loop_ == nullptr);
    loop_ = loop;
    runtimeAssert(loop_->isInLoopThread());
    channel_->setLoop(loop_);
    status_ = TcpConnection::kConnected;
    channel_->enableReadEvent();
    if (type_ == kServer) {
        if (TlsOptions::instance().isTlsAuthClients()) {
            SSL_set_verify(ssl_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
        } else {
            SSL_set_verify(ssl_, SSL_VERIFY_NONE, nullptr);
        }
        SSL_set_accept_state(ssl_);
        ssl_status_ = kAccepting;
    } else if (type_ == kClient) {
        SSL_set_connect_state(ssl_);
        ssl_status_ = kConnecting;
        sslConnect();
    }
    if (connection_callback_) {
        connection_callback_(shared_from_this());
    }
}

// @override
void TlsConnection::handleRead() {
    runtimeAssert(loop_->isInLoopThread());
    runtimeAssert(ssl_status_ != kNone);
    if (before_read_event_callback_ && !before_read_event_callback_(shared_from_this())) {
        return;
    }
    std::shared_ptr<TcpConnection> _ = shared_from_this(); // Holds shared_ptr, avoid being destroyed in the callback function.

    if (ssl_status_ == kAccepting) {
        sslAccept();
    }
    if (ssl_status_ == kConnecting) {
        sslConnect();
    }
    if (ssl_status_ == kConnected && ssl_write_want_read_) {
        ssl_write_want_read_ = false;
        sslWrite();
    }
    if (ssl_status_ == kConnected) {
        ssl_read_want_write_ = false;
        sslRead();
    }
}

// @override
void TlsConnection::handleWrite() {
    runtimeAssert(loop_->isInLoopThread());
    runtimeAssert(ssl_status_ != kNone);
    if (before_write_event_callback_ && !before_write_event_callback_(shared_from_this())) {
        return;
    }
    std::shared_ptr<TcpConnection> _ = shared_from_this(); // Holds shared_ptr, avoid being destroyed in the callback function.

    if (ssl_status_ == kAccepting) {
        sslAccept();
    }
    if (ssl_status_ == kConnecting) {
        sslConnect();
    }
    if (ssl_status_ == kConnected && ssl_read_want_write_) {
        ssl_read_want_write_ = false;
        sslRead();
    }
    if (ssl_status_ == kConnected) {
        ssl_write_want_read_ = false;
        sslWrite();
    }
}

// @override
void TlsConnection::handleError() {
    runtimeAssert(loop_->isInLoopThread());
    ssl_status_ = kDisconnected;
    TcpConnection::handleError();
}

// @override
void TlsConnection::handleClose() {
    runtimeAssert(loop_->isInLoopThread());
    SSL_free(ssl_);
    ssl_ = nullptr;
    ssl_status_ = kDisconnected;
    LOG_DEBUG("TlsConnection closed, addr={} fd={}", remote_ip_port_, fd_);
    TcpConnection::handleClose();
}

// ssl object may be freed in this function.
int TlsConnection::sslError(int ret_code) {
    int ssl_err = SSL_get_error(ssl_, ret_code);
    switch (ssl_err) {
        case SSL_ERROR_WANT_WRITE:
            channel_->enableWriteEvent();
            break;
        case SSL_ERROR_WANT_READ:
            channel_->enableReadEvent();
            break;
        case SSL_ERROR_ZERO_RETURN:
            LOG_DEBUG("SSL_ERROR_SYSCALL: closed by peer.");
            handleError();
            break;
        case SSL_ERROR_SYSCALL:
            if (errno == 0)
                LOG_DEBUG("SSL_ERROR_SYSCALL: closed by peer.");
            else
                LOG_DEBUG("SSL_ERROR_SYSCALL, errno: {}.", SystemUtil::errnoToString(errno));
            handleError();
            break;
        default:
            char error_str[256];
            ERR_error_string_n(ERR_get_error(), error_str, 256);
            LOG_DEBUG("SSL_get_error code: {}, errors:{}", ssl_err, error_str);
            handleError();
            break;
    }
    return ssl_err;
}

void TlsConnection::sslAccept() {
    runtimeAssert(ssl_status_ == kAccepting);

    ERR_clear_error();
    int ret = SSL_accept(ssl_);
    if (ret <= 0) {
        sslError(ret);
    } else {
        ssl_status_ = kConnected;
    }
}

void TlsConnection::sslConnect() {
    runtimeAssert(ssl_status_ == kConnecting);

    ERR_clear_error();
    int ret = SSL_connect(ssl_);
    if (ret <= 0) {
        sslError(ret);
    } else {
        ssl_status_ = kConnected;
    }
}

void TlsConnection::sslRead() {
    std::shared_ptr<TcpConnection> _ = shared_from_this(); // Holds shared_ptr, avoid being destroyed in the callback function.

    ERR_clear_error();
    input_buffer_.ensureWritableBytes(PROTO_IOBUF_LEN);
    int ret = SSL_read(ssl_, input_buffer_.writeBegin(), PROTO_IOBUF_LEN);
    if (ret > 0) {
        int nread = ret;
        // check ssl buffer
        if (!has_pending_read_in_loop_ && SSL_pending(ssl_)) {
            // loop_ will holds the conn, until this callback run in loop.
            auto expected_cb = [conn = shared_from_this()]() {
                return conn->loop();
            };
            loop_->queueInLoopMaybeRedir(expected_cb, [that = std::dynamic_pointer_cast<TlsConnection>(shared_from_this())](EventLoop *) {
                that->has_pending_read_in_loop_ = false;
                that->handleRead();
            });
            has_pending_read_in_loop_ = true;
        }
        input_buffer_.incrWriteIndex(nread);
        NetworkStat::addNetInputBytes(nread);
        // call onMessage
        if (message_callback_) {
            message_callback_(shared_from_this(), &input_buffer_);
        } else {
            input_buffer_.reset();
        }
    } else {
        int ssl_err = sslError(ret);
        if (ssl_err == SSL_ERROR_WANT_WRITE) {
            ssl_read_want_write_ = true;
        }
    }
}

void TlsConnection::sslWrite() {
    std::shared_ptr<TcpConnection> _ = shared_from_this(); // Holds shared_ptr, avoid being destroyed in the callback function.

    if (output_buffer_.length() == 0) {
        if (!ssl_read_want_write_) { // Both sslWrite() and sslRead() do not need write event to fire.
            channel_->disableWriteEvent();
        }
        return;
    }

    ERR_clear_error();
    int ret = SSL_write(ssl_, output_buffer_.data(), (int)output_buffer_.length());
    if (ret > 0) {
        NetworkStat::addNetOutputBytes(ret);
        output_buffer_.skip(ret);
        if (output_buffer_.empty()) {
            if (!ssl_read_want_write_) { // Both sslWrite() and sslRead() do not need write event to fire.
                channel_->disableWriteEvent();
            }
            if (write_complete_callback_) {
                write_complete_callback_(shared_from_this());
            }
        }
    } else {
        int ssl_err = sslError(ret);
        if (ssl_err == SSL_ERROR_WANT_READ) {
            ssl_write_want_read_ = true;
        }
    }
}

void TlsConnection::sendInLoop(const void *data, size_t len) {
    std::shared_ptr<TcpConnection> _ = shared_from_this(); // holds shared_ptr, avoid being destroyed in the callback function.

    if (len == 0) return;                                // should not call SSL_write() with num=0, it will return an error.
    if (ssl_status_ == kDisconnected) return;            // conn may be closed.
    runtimeAssert(status_ == TcpConnection::kConnected); // if ssl_status != disconnected, then status == connected.

    int nwritten = 0;
    size_t remaining = len;
    // if no data in output queue, writing directly
    if (!channel_->hasWritableEvent() && output_buffer_.empty()) {
        ERR_clear_error();
        int ret = SSL_write(ssl_, static_cast<const char *>(data), (int)len);
        if (ret > 0) {
            nwritten = ret;
            NetworkStat::addNetOutputBytes(nwritten);
            remaining = len - nwritten;
            if (remaining == 0 && write_complete_callback_) {
                write_complete_callback_(shared_from_this());
            }
        } else {
            int ssl_err = sslError(ret);
            if (ssl_err == SSL_ERROR_WANT_READ) {
                ssl_write_want_read_ = true;
            }
        }
    }

    if (remaining > 0 && ssl_status_ != kDisconnected) { // conn may be closed after SSL_write.
        size_t old_len = output_buffer_.length();
        size_t current_size = old_len + remaining;
        if (current_size >= high_water_mark_ && old_len < high_water_mark_) {
            LOG_TRACE("Connection high water, current size: {}", current_size);
            if (high_water_mark_callback_) {
                high_water_mark_callback_(shared_from_this(), old_len + remaining);
            }
        }
        output_buffer_.append((char *)data + nwritten, remaining);
        if (!channel_->hasWritableEvent()) {
            channel_->enableWriteEvent();
        }
    }
}

bool TlsConnection::isTLSConnection() const {
    return true;
}

} // namespace tair::network
