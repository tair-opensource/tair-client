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
#include "client/TairClient.hpp"
#include "client/TairResult.hpp"
#include "common/CountDownLatch.hpp"

#include <iostream>

using tair::client::TairClient;
using tair::client::TairURI;
using tair::client::TairResult;
using tair::common::CountDownLatch;

int main(int argc, char *argv[]) {
    // connect
    auto *client = new TairClient();
    TairURI uri = TairURI::create()
            .type(TairURI::STANDALONE)
            .serverAddrs({"127.0.0.1:6379"})
            .connectingTimeoutMs(3000)
            .build();
    if (!client->init(uri).isSuccess()) {
        std::cerr << "Connect error, see logs for detail" << std::endl;
    }

    // sync usage
    auto wrapper = client->getFutureWrapper();
    std::future<TairResult<std::string>> f = wrapper.set("key", "value");
    if (f.get().isSuccess()) {
        std::cout << "set key success" << std::endl;
    } else {
        std::cerr << "set key fail" << std::endl;
    }

    // async usage, we need a latch to pending
    CountDownLatch latch;
    client->get("key", [&](const TairResult<std::shared_ptr<std::string>> &result) {
        latch.countDown();
        if (result.isSuccess()) {
            std::cout << "get key: " << *(result.getValue()) << std::endl;
        } else {
            std::cerr << "get key fail" << std::endl;
        }
    });
    latch.wait();
    
    // destroy
    client->destroy();
}