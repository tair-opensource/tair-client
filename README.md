# Tair C++ Client

Tair C++ Client is a client used to connect to Tair, it can also be used to connect to Redis, because Tair is fully compatible with Redis.

- Asynchronous and provides a synchronous future interface
- Support Standalone and Cluster
- Support seamless switching capabilities [WIP]
- Support rust, go, python interfaces [WIP]

# How to Use

1. build deps
```
./bootstrap.sh deps release[debug]
```

2. build source
```
./bootstrap.sh build release[debug]
```

3. [optional] start redis at `tests/ut_tests/client/TairClient_Standalone_Server.cpp#STANDALONE_ADDR`(default: 127.0.0.1:6379) and run test
```
./bootstrap.sh test
```

# Example
```
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
```
For more examples please refer to the examples directory.