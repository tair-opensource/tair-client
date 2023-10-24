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
#include "network/TlsOptions.hpp"

#include "common/Logger.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>

namespace tair::network {

bool TlsOptions::configure(const TlsConfig &config) {
    // using OpenSSL 1.1.0 or above
    runtimeAssert(OPENSSL_VERSION_NUMBER >= 0x10101000L);

    int protocols = 0;
    char errbuf[512] = {};

    if (config.tls_cert_file.empty()) {
        LOG_ERROR("No tls-cert-file configured!");
        return false;
    }
    if (config.tls_key_file.empty()) {
        LOG_ERROR("No tls-key-file configured!");
        return false;
    }

    std::shared_ptr<SSL_CTX> ctx(SSL_CTX_new(TLS_method()), [](SSL_CTX *ssl_ctx) { ::SSL_CTX_free(ssl_ctx); });

    SSL_CTX_set_options(ctx.get(), SSL_OP_ALL);
    if (config.tls_session_caching != 0) {
        SSL_CTX_set_session_cache_mode(ctx.get(), SSL_SESS_CACHE_SERVER);
        SSL_CTX_sess_set_cache_size(ctx.get(), config.tls_session_cache_size);
        SSL_CTX_set_timeout(ctx.get(), config.tls_session_cache_timeout);
        SSL_CTX_set_session_id_context(ctx.get(), (unsigned char *)"Tair", 4);
    } else {
        SSL_CTX_set_session_cache_mode(ctx.get(), SSL_SESS_CACHE_OFF);
    }

    protocols = parseProtocolsConfig(config.tls_protocols);
    if (protocols == 0) {
        LOG_ERROR("Wrong tls_protocols: {}", config.tls_protocols);
        return false;
    }
    /*
     *  Clients should avoid creating "holes" in the set of protocols
     *  they support. When disabling a protocol, make sure that you also
     *  disable either all previous or all subsequent protocol versions.
     *  In clients, when a protocol version is disabled without disabling
     *  all previous protocol versions, the effect is to also disable
     *  all subsequent protocol versions.
     * */
    SSL_CTX_set_min_proto_version(ctx.get(), SSL3_VERSION);
    if ((protocols & REDIS_TLS_PROTO_TLSv1) == 0) {
        SSL_CTX_set_options(ctx.get(), SSL_OP_NO_TLSv1);
    }
    if ((protocols & REDIS_TLS_PROTO_TLSv1_1) == 0) {
        SSL_CTX_set_options(ctx.get(), SSL_OP_NO_TLSv1_1);
    }
    if ((protocols & REDIS_TLS_PROTO_TLSv1_2) == 0) {
        SSL_CTX_set_options(ctx.get(), SSL_OP_NO_TLSv1_2);
    }
    if ((protocols & REDIS_TLS_PROTO_TLSv1_3) == 0) {
        SSL_CTX_set_options(ctx.get(), SSL_OP_NO_TLSv1_3);
    }

    // TLS Compression is dangerous and should be avoided on the public internet.
    SSL_CTX_set_options(ctx.get(), SSL_OP_NO_COMPRESSION);

    if (config.tls_prefer_server_ciphers) {
        SSL_CTX_set_options(ctx.get(), SSL_OP_CIPHER_SERVER_PREFERENCE);
    }

    SSL_CTX_set_mode(ctx.get(), SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_AUTO_RETRY);

    // load cert file.
    if (SSL_CTX_use_certificate_chain_file(ctx.get(), config.tls_cert_file.data()) <= 0) {
        ERR_error_string_n(ERR_get_error(), errbuf, sizeof(errbuf));
        LOG_ERROR("Failed to load certificate: {}: {}", config.tls_cert_file.data(), errbuf);
        return false;
    }

    // load key file.
    if (SSL_CTX_use_PrivateKey_file(ctx.get(), config.tls_key_file.data(), SSL_FILETYPE_PEM) <= 0) {
        ERR_error_string_n(ERR_get_error(), errbuf, sizeof(errbuf));
        LOG_ERROR("Failed to load private key: {}: {}", config.tls_key_file, errbuf);
        return false;
    }

    // load ca file.
    if (config.tls_auth_clients && SSL_CTX_load_verify_locations(ctx.get(), config.tls_ca_file.data(), nullptr) <= 0) {
        ERR_error_string_n(ERR_get_error(), errbuf, sizeof(errbuf));
        LOG_ERROR("Failed to configure CA certificate(s) file/directory: {}", errbuf);
        return false;
    }

    LockGuard lock(mutex_);
    ssl_ctx_ = ctx;
    tls_auth_clients_ = config.tls_auth_clients;

    return true;
}

int TlsOptions::parseProtocolsConfig(const std::string &str) {
    if (str.empty()) {
        return REDIS_TLS_PROTO_DEFAULT;
    }
    int protocols = 0;
    if (str.find("TLSv1") != std::string::npos) {
        protocols |= REDIS_TLS_PROTO_TLSv1;
    }
    if (str.find("TLSv1.1") != std::string::npos) {
        protocols |= REDIS_TLS_PROTO_TLSv1_1;
    }
    if (str.find("TLSv1.2") != std::string::npos) {
        protocols |= REDIS_TLS_PROTO_TLSv1_2;
    }
    if (str.find("TLSv1.3") != std::string::npos) {
        protocols |= REDIS_TLS_PROTO_TLSv1_3;
    }
    return protocols;
}

void TlsOptions::clear() {
    LockGuard lock(mutex_);
    ssl_ctx_.reset();
    tls_auth_clients_ = false;
}

} // namespace tair::network
