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

#include <memory>
#include <string>

#include "common/Assert.hpp"
#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"

typedef struct ssl_ctx_st SSL_CTX;

namespace tair::network {

using common::Mutex;
using common::LockGuard;
using common::Noncopyable;

#define REDIS_TLS_PROTO_TLSv1   (1 << 0)
#define REDIS_TLS_PROTO_TLSv1_1 (1 << 1)
#define REDIS_TLS_PROTO_TLSv1_2 (1 << 2)
#define REDIS_TLS_PROTO_TLSv1_3 (1 << 3)
/* Use safe defaults */
#define REDIS_TLS_PROTO_DEFAULT (REDIS_TLS_PROTO_TLSv1_2 | REDIS_TLS_PROTO_TLSv1_3)

struct TlsConfig {
    std::string tls_protocols; // both

    std::string tls_key_file;  // server
    std::string tls_cert_file; // server

    bool tls_auth_clients;
    std::string tls_ca_file; // client

    bool tls_session_caching;
    int64_t tls_session_cache_size;
    long tls_session_cache_timeout;
    bool tls_prefer_server_ciphers;
};

class TlsOptions : private Noncopyable {
public:
    static TlsOptions &instance() {
        static TlsOptions options;
        return options;
    }

private:
    TlsOptions() = default;

public:
    ~TlsOptions() = default;

    bool isConfigured() EXCLUDES(mutex_) {
        LockGuard lock(mutex_);
        return ssl_ctx_ != nullptr;
    }

    bool configure(const TlsConfig &config) EXCLUDES(mutex_);
    void clear() EXCLUDES(mutex_);

    std::shared_ptr<SSL_CTX> getSslContext() EXCLUDES(mutex_) {
        LockGuard lock(mutex_);
        runtimeAssert(ssl_ctx_ != nullptr);
        return ssl_ctx_;
    }

    bool isTlsAuthClients() const EXCLUDES(mutex_) {
        LockGuard lock(mutex_);
        return tls_auth_clients_;
    }

private:
    static int parseProtocolsConfig(const std::string &str);

private:
    mutable Mutex mutex_;
    std::shared_ptr<SSL_CTX> ssl_ctx_ GUARDED_BY(mutex_);
    bool tls_auth_clients_ GUARDED_BY(mutex_) = false;
};

} // namespace tair::network
