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

class SortParams : public Params {
public:
    SortParams &by(const std::string &by_pattern) {
        addParams(KeyWord::BY);
        by_pattern_ = by_pattern;
        return *this;
    }

    SortParams &limit(int64_t offset, int64_t count) {
        addParams(KeyWord::LIMIT);
        offset_ = offset;
        count_ = count;
        return *this;
    }

    SortParams &get(const std::vector<std::string> &get_pattern) {
        addParams(KeyWord::GET);
        get_pattern_ = get_pattern;
        return *this;
    }

    SortParams &asc() {
        addParams(KeyWord::ASC);
        return *this;
    }

    SortParams &desc() {
        addParams(KeyWord::DESC);
        return *this;
    }

    SortParams &alpha() {
        addParams(KeyWord::ALPHA);
        return *this;
    }

    void addParamsToArgv(std::vector<std::string> &argv) const override {
        if (containParams(KeyWord::BY)) {
            argv.emplace_back("BY");
            argv.emplace_back(by_pattern_);
        }
        if (containParams(KeyWord::LIMIT)) {
            argv.emplace_back("LIMIT");
            argv.emplace_back(std::to_string(offset_));
            argv.emplace_back(std::to_string(count_));
        }
        if (containParams(KeyWord::GET)) {
            for (auto &g : get_pattern_) {
                argv.emplace_back("GET");
                argv.emplace_back(g);
            }
        }
        if (containParams(KeyWord::ASC)) {
            argv.emplace_back("ASC");
        }
        if (containParams(KeyWord::DESC)) {
            argv.emplace_back("DESC");
        }
        if (containParams(KeyWord::ALPHA)) {
            argv.emplace_back("ALPHA");
        }
    }

private:
    std::string by_pattern_;
    int64_t offset_;
    int64_t count_;
    std::vector<std::string> get_pattern_;
};

} // namespace tair::client
