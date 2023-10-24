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

class ZRangeParams : public Params {
public:
    ZRangeParams &byscore() {
        addParams(KeyWord::BYSCORE);
        return *this;
    }

    ZRangeParams &bylex() {
        addParams(KeyWord::BYLEX);
        return *this;
    }

    ZRangeParams &rev() {
        addParams(KeyWord::REV);
        return *this;
    }

    ZRangeParams &limit(int64_t offset, int64_t count) {
        addParams(KeyWord::LIMIT);
        offset_ = offset;
        count_ = count;
        return *this;
    }

    ZRangeParams &withscores() {
        addParams(KeyWord::WITHSCORES);
        return *this;
    }

    void addParamsToArgv(std::vector<std::string> &argv) const override {
        if (containParams(KeyWord::BYSCORE)) {
            argv.emplace_back("BYSCORE");
        }
        if (containParams(KeyWord::BYLEX)) {
            argv.emplace_back("BYLEX");
        }
        if (containParams(KeyWord::REV)) {
            argv.emplace_back("REV");
        }
        if (containParams(KeyWord::LIMIT)) {
            argv.emplace_back("LIMIT");
            argv.emplace_back(std::to_string(offset_));
            argv.emplace_back(std::to_string(count_));
        }
        if (containParams(KeyWord::WITHSCORES)) {
            argv.emplace_back("WITHSCORES");
        }
    }

private:
    int64_t offset_;
    int64_t count_;
};

} // namespace tair::client
