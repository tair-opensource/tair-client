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

class RestoreParams : public Params {
public:
    RestoreParams &replace() {
        addParams(KeyWord::REPLACE);
        return *this;
    }

    RestoreParams &absttl() {
        addParams(KeyWord::ABSTTL);
        return *this;
    }

    RestoreParams &idletime(int64_t idletime) {
        addParams(KeyWord::IDLETIME);
        idletime_ = idletime;
        return *this;
    }

    RestoreParams &freq(int64_t freq) {
        addParams(KeyWord::FREQ);
        freq_ = freq;
        return *this;
    }

    void addParamsToArgv(std::vector<std::string> &argv) const override {
        if (containParams(KeyWord::REPLACE)) {
            argv.emplace_back("REPLACE");
        }
        if (containParams(KeyWord::ABSTTL)) {
            argv.emplace_back("ABSTTL");
        }
        if (containParams(KeyWord::IDLETIME)) {
            argv.emplace_back("IDLETIME");
            argv.emplace_back(std::to_string(idletime_));
        }
        if (containParams(KeyWord::FREQ)) {
            argv.emplace_back("FREQ");
            argv.emplace_back(std::to_string(freq_));
        }
    }

private:
    int64_t idletime_;
    int64_t freq_;
};

} // namespace tair::client
