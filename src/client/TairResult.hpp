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

#include <string>

namespace tair::client {

constexpr const static char *E_NOT_INIT = "tair client not init";
constexpr const static char *E_PARAMS_EMPTY = "api params is empty or not enough";
constexpr const static char *E_NOT_IN_SAME_SLOT = "multiple keys are not in the same slot";
constexpr const static char *E_CLUSTER_NOT_SUPPORT_COMMAND = "cluster mode do not support this command";

template <typename T>
class TairResult {
public:
    TairResult() = default;
    ~TairResult() = default;

    template <typename U>
    static TairResult<T> createErr(U &&u) {
        TairResult<T> result;
        result.setErr(std::forward<U>(u));
        return result;
    }

    static TairResult<T> create(T &&t) {
        TairResult<T> result;
        result.setValue(std::forward<T>(t));
        return result;
    }

    bool isSuccess() const {
        return err_.empty();
    }

    const T &getValue() const {
        return value_;
    }

    void setValue(T t) {
        value_ = t;
    }

    const std::string &getErr() const {
        return err_;
    }

    void setErr(const std::string &err) {
        err_ = err;
    }

private:
    T value_;
    std::string err_;
};

} // namespace tair::client
