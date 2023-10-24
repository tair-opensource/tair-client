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

#include "client/params/KeyWord.hpp"
#include "client/params/Params.hpp"

namespace tair::client {

class BitPositonParams : public Params {
public:
    BitPositonParams &start(int64_t start) {
        addParams(KeyWord::START);
        start_ = start;
        return *this;
    }

    BitPositonParams &end(int64_t end) {
        addParams(KeyWord::END);
        end_ = end;
        return *this;
    }

    BitPositonParams &byte() {
        addParams(KeyWord::BYTE);
        return *this;
    }

    BitPositonParams &bit() {
        addParams(KeyWord::BIT);
        return *this;
    }

    void addParamsToArgv(std::vector<std::string> &argv) const override {
        if (containParams(KeyWord::START)) {
            argv.emplace_back(std::to_string(start_));
        }
        if (containParams(KeyWord::END)) {
            argv.emplace_back(std::to_string(end_));
        }
        if (containParams(KeyWord::BYTE)) {
            argv.emplace_back("BYTE");
        }
        if (containParams(KeyWord::BIT)) {
            argv.emplace_back("BIT");
        }
    }

private:
    int64_t start_;
    int64_t end_;
};

} // namespace tair::client
