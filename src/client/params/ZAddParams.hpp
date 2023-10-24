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

class ZAddParams : public Params {
public:
    ZAddParams &nx() {
        addParams(KeyWord::NX);
        return *this;
    }

    ZAddParams &xx() {
        addParams(KeyWord::XX);
        return *this;
    }

    ZAddParams &gt() {
        addParams(KeyWord::GT);
        return *this;
    }

    ZAddParams &lt() {
        addParams(KeyWord::LT);
        return *this;
    }

    ZAddParams &ch() {
        addParams(KeyWord::CH);
        return *this;
    }

    // note: we do not support incr
    // pls use zincrby command

    void addParamsToArgv(std::vector<std::string> &argv) const override {
        if (containParams(KeyWord::NX)) {
            argv.emplace_back("NX");
        }
        if (containParams(KeyWord::XX)) {
            argv.emplace_back("XX");
        }
        if (containParams(KeyWord::GT)) {
            argv.emplace_back("GT");
        }
        if (containParams(KeyWord::LT)) {
            argv.emplace_back("LT");
        }
        if (containParams(KeyWord::CH)) {
            argv.emplace_back("CH");
        }
    }
};

} // namespace tair::client
