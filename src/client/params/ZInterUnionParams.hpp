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

#include <initializer_list>
#include <vector>

namespace tair::client {

class ZInterUnionParams : public Params {
public:
    ZInterUnionParams &weights(std::initializer_list<double> weights) {
        addParams(KeyWord::WEIGHTS);
        weights_ = weights;
        return *this;
    }

    ZInterUnionParams &aggregate(const std::string &type) {
        addParams(KeyWord::AGGREGATE);
        aggregate_ = type;
        return *this;
    }

    void addParamsToArgv(std::vector<std::string> &argv) const override {
        if (containParams(KeyWord::WEIGHTS)) {
            argv.emplace_back("WEIGHTS");
            for (auto weight : weights_) {
                argv.emplace_back(std::to_string(weight));
            }
        }
        if (containParams(KeyWord::AGGREGATE)) {
            argv.emplace_back("AGGREGATE");
            argv.emplace_back(aggregate_);
        }
    }

private:
    std::vector<double> weights_;
    std::string aggregate_;
};

} // namespace tair::client
