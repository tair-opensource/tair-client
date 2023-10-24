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

#include "network/TcpConnection.hpp"
#include "network/TlsOptions.hpp"

typedef struct ssl_st SSL;

namespace tair::network {

#define PROTO_IOBUF_LEN (1024 * 16) // Generic I/O buffer size

class TlsConnection final : public TcpConnection {
public:
    enum Type {
        kServer = 0,
        kClient = 1,
    };

    TlsConnection(socket_t sockfd, const std::string &local_ip_port,
                  const std::string &remote_ip_port, Type type);

    ~TlsConnection() override = default;

private:
    enum SSL_STATUS {
        kNone = 0,
        kAccepting = 1,
        kConnecting = 2,
        kConnected = 3,
        kDisconnected = 4,
    };

    void attachedToLoop(EventLoop *loop) override;

    void handleRead() override;
    void handleWrite() override;
    void handleError() override;
    void handleClose() override;

    void sslAccept();
    void sslConnect();
    void sslRead();
    void sslWrite();
    int sslError(int ret_code);

    void sendInLoop(const void *data, size_t len) override;

    bool isTLSConnection() const override;

private:
    SSL *ssl_ = nullptr;
    Type type_ = kServer;
    SSL_STATUS ssl_status_ = kNone;
    bool ssl_write_want_read_ = false;
    bool ssl_read_want_write_ = false;
    bool has_pending_read_in_loop_ = false;
};

} // namespace tair::network
