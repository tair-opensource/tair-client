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
#include "TairTransactionClient.hpp"

#include "client/TairResultHelper.hpp"

#include <iostream>
namespace tair::client {

using protocol::SimpleStringPacket;

void TairTransactionClient::multi() {
    appendCommand({"multi"});
}

void TairTransactionClient::exec(const ResultPacketCallback &callback) {
    while (!cmd_stack_.empty()) {
        sendCommand(cmd_stack_.front(), [](auto &, auto &, int64_t) {
            // ignore
        });
        cmd_stack_.pop_front();
    }
    sendCommand({"exec"}, [callback](auto &req, auto &resp, int64_t) {
        callback(nullptr, req, resp);
    });
}

void TairTransactionClient::watch(std::initializer_list<std::string> keys, const ResultStringCallback &callback) {
    CommandArgv argv {"watch"};
    argv.insert(argv.end(), keys);

    sendCommand(std::move(argv), [callback](auto &, auto &resp, int64_t) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairTransactionClient::unwatch(const ResultStringCallback &callback) {
    sendCommand({"unwatch"}, [callback](auto &, auto &resp, int64_t) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairTransactionClient::discard(const ResultStringCallback &callback) {
    sendCommand({"discard"}, [callback](auto &, auto &resp, int64_t) {
        TairResultHelper::doCallbackOneResult<SimpleStringPacket, std::string>(resp, callback);
    });
}

void TairTransactionClient::appendCommand(CommandArgv &argv) {
    cmd_stack_.emplace_back(argv);
}

void TairTransactionClient::appendCommand(CommandArgv &&argv) {
    cmd_stack_.emplace_back(argv);
}

} // namespace tair::client