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

class ScanParams : public Params {
public:
    ScanParams &match(const std::string &match) {
        addParams(KeyWord::MATCH);
        match_ = match;
        return *this;
    }

    ScanParams &count(int64_t count) {
        addParams(KeyWord::COUNT);
        count_ = count;
        return *this;
    }

    ScanParams &type(const std::string &type) {
        addParams(KeyWord::TYPE);
        type_ = type;
        return *this;
    }

    void addParamsToArgv(std::vector<std::string> &argv) const override {
        if (containParams(KeyWord::MATCH)) {
            argv.emplace_back("MATCH");
            argv.emplace_back(match_);
        }
        if (containParams(KeyWord::COUNT)) {
            argv.emplace_back("COUNT");
            argv.emplace_back(std::to_string(count_));
        }
        if (containParams(KeyWord::TYPE)) {
            argv.emplace_back("TYPE");
            argv.emplace_back(type_);
        }
    }

private:
    std::string match_;
    int64_t count_;
    std::string type_;
};

} // namespace tair::client
