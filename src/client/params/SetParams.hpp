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

class SetParams : public Params {
public:
    SetParams &nx() {
        addParams(KeyWord::NX);
        return *this;
    }

    SetParams &xx() {
        addParams(KeyWord::XX);
        return *this;
    }

    SetParams &keepttl() {
        addParams(KeyWord::KEEPTTL);
        return *this;
    }

    SetParams &ex(int64_t ex) {
        ex_ = ex;
        addParams(KeyWord::EX);
        return *this;
    }

    SetParams &px(int64_t px) {
        px_ = px;
        addParams(KeyWord::PX);
        return *this;
    }

    SetParams &exat(int64_t exat) {
        exat_ = exat;
        addParams(KeyWord::EXAT);
        return *this;
    }

    SetParams &pxat(int64_t pxat) {
        pxat_ = pxat;
        addParams(KeyWord::PXAT);
        return *this;
    }

    void addParamsToArgv(std::vector<std::string> &argv) const override {
        if (containParams(KeyWord::NX)) {
            argv.emplace_back("NX");
        }
        if (containParams(KeyWord::XX)) {
            argv.emplace_back("XX");
        }
        if (containParams(KeyWord::KEEPTTL)) {
            argv.emplace_back("KEEPTTL");
        }
        if (containParams(KeyWord::EX)) {
            argv.emplace_back("EX");
            argv.emplace_back(std::to_string(ex_));
        }
        if (containParams(KeyWord::PX)) {
            argv.emplace_back("PX");
            argv.emplace_back(std::to_string(px_));
        }
        if (containParams(KeyWord::EXAT)) {
            argv.emplace_back("EXAT");
            argv.emplace_back(std::to_string(exat_));
        }
        if (containParams(KeyWord::PXAT)) {
            argv.emplace_back("PXAT");
            argv.emplace_back(std::to_string(pxat_));
        }
    }

private:
    int64_t ex_;
    int64_t px_;
    int64_t exat_;
    int64_t pxat_;
};

} // namespace tair::client
