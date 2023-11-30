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

#include "gtest/gtest.h"

#include <thread>

#include "common/CountDownLatch.hpp"
#include "network/TcpServer.hpp"
#include "client/TairClient.hpp"
#include "client/TairURI.hpp"

using tair::common::CountDownLatch;
using tair::client::TairClient;
using tair::client::TairURI;

extern std::string STANDALONE_ADDR;
constexpr static bool use_external_server = false;

class StandAloneTest : public ::testing::Test {
protected:
    // Per test set-up
    void SetUp() override {
        // Init Client
        client = std::make_unique<TairClient>();
        TairURI uri = TairURI::create()
                          .type(TairURI::STANDALONE)
                          .serverAddrs({STANDALONE_ADDR})
                          .connectingTimeoutMs(3000)
                          .build();
        ASSERT_TRUE(client->init(uri).isSuccess());
        // Flushall
        ASSERT_EQ("OK", client->getFutureWrapper().flushall().get().getValue());
    }

    // Per test tear-down
    void TearDown() override {
        client->destroy();
    }

public:
    static std::unique_ptr<TairClient> client;

private:
    static std::unique_ptr<std::thread> server_thread_;
};
