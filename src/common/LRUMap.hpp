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

#include <list>
#include <unordered_map>

#include "common/Copyable.hpp"

namespace tair::common {

template <typename K, typename V>
class LRUMap : public Copyable {
public:
    explicit LRUMap(size_t capacity) {
        capacity_ = capacity;
    }

    using iterator = typename std::unordered_map<K, V>::iterator;

    iterator get(K key) {
        auto it = kv_map_.find(key);
        if (it == kv_map_.end()) {
            return it;
        }
        auto ki = ki_map_.find(key);
        k_list_.splice(k_list_.begin(), k_list_, ki->second);
        return it;
    }

    void put(K key, V value) {
        if (kv_map_.find(key) != kv_map_.end()) {
            kv_map_[key] = value;
            auto ki = ki_map_.find(key);
            k_list_.splice(k_list_.begin(), k_list_, ki->second);
            return;
        }
        kv_map_[key] = value;
        k_list_.emplace_front(key);
        ki_map_[key] = k_list_.begin();
        if (kv_map_.size() > capacity_) {
            kv_map_.erase(k_list_.back());
            ki_map_.erase(k_list_.back());
            k_list_.pop_back();
        }
    }

    void clear() {
        kv_map_.clear();
        ki_map_.clear();
        k_list_.clear();
    }

    void size() const {
        return ki_map_.size();
    }

private:
    size_t capacity_;
    std::unordered_map<K, V> kv_map_;
    std::unordered_map<K, typename std::list<K>::iterator> ki_map_;
    std::list<K> k_list_;
};

} // namespace tair::common
