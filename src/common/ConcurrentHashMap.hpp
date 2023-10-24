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

#include <functional>
#include <unordered_map>
#include <vector>

#include "common/Mutex.hpp"
#include "common/Noncopyable.hpp"

namespace tair::common {

template <typename Key, typename Value, size_t SLOT_COUNT = 64, typename Hash = std::hash<Key>>
class ConcurrentHashMap : private Noncopyable {
public:
    ConcurrentHashMap() = default;
    ~ConcurrentHashMap() = default;

    using InnerMap = std::unordered_map<Key, Value, Hash>;
    using Iterator = typename InnerMap::iterator;
    using ValueType = typename InnerMap::value_type;
    using ValueFilter = std::function<bool(const Value &value)>;

    std::pair<Iterator, bool> insert(const ValueType &v) {
        size_t index = hasher_(v.first) % SLOT_COUNT;
        LockGuard lock(mutexs_[index]);
        return hashmaps_[index].insert(v);
    }

    std::pair<Iterator, bool> insert(ValueType &&v) {
        size_t index = hasher_(v.first) % SLOT_COUNT;
        LockGuard lock(mutexs_[index]);
        return hashmaps_[index].insert(std::move(v));
    }

    std::tuple<bool, Value> get(const Key &key) {
        size_t index = hasher_(key) % SLOT_COUNT;
        LockGuard lock(mutexs_[index]);
        auto iter = hashmaps_[index].find(key);
        if (iter != hashmaps_[index].end()) {
            return std::make_tuple(true, iter->second);
        } else {
            return std::make_tuple(false, Value());
        }
    }

    std::vector<Value> getValues(const ValueFilter &filter) {
        std::vector<Value> values;
        for (size_t index = 0; index < SLOT_COUNT; ++index) {
            LockGuard lock(mutexs_[index]);
            for (auto &[_, value] : hashmaps_[index]) {
                if (filter(value)) {
                    values.template emplace_back(value);
                }
            }
        }
        return values;
    }

    size_t erase(const Key &key) {
        size_t index = hasher_(key) % SLOT_COUNT;
        LockGuard lock(mutexs_[index]);
        return hashmaps_[index].erase(key);
    }

    bool contains(const Key &key) const {
        size_t index = hasher_(key) % SLOT_COUNT;
        LockGuard lock(mutexs_[index]);
        return hashmaps_[index].contains(key);
    }

    size_t size() const {
        size_t n = 0;
        for (size_t index = 0; index < SLOT_COUNT; ++index) {
            LockGuard lock(mutexs_[index]);
            n += hashmaps_[index].size();
        }
        return n;
    }

    void clear() {
        for (size_t index = 0; index < SLOT_COUNT; ++index) {
            LockGuard lock(mutexs_[index]);
            hashmaps_[index].clear();
        }
    }

private:
    mutable Mutex mutexs_[SLOT_COUNT];
    InnerMap hashmaps_[SLOT_COUNT];
    Hash hasher_;
};

} // namespace tair::common
