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
#include "gtest/gtest.h"

#include <deque>
#include <set>
#include <unordered_map>
#include <vector>

TEST(VECTOR_TEST, ONLY_TEST) {
    std::vector<int> vect;
    fprintf(stderr, "vector size:%lu, capacity:%lu\n", vect.size(), vect.capacity());
    for (int i = 0; i < 10; ++i) {
        vect.emplace_back(1);
        fprintf(stderr, "vector size:%lu, capacity:%lu\n", vect.size(), vect.capacity());
    }
}

TEST(SET_TEST, ONLY_TEST) {
    struct A {
        A(int64_t ms)
            : ms_(ms) {}
        int64_t ms_;
    };
    using APtr = std::shared_ptr<A>;

    struct SetCompare {
        bool operator()(const APtr &lhs, const APtr &rhs) const {
            if (lhs->ms_ != rhs->ms_) {
                return lhs->ms_ < rhs->ms_;
            } else {
                return lhs.get() < rhs.get();
            }
        }
    };
    std::set<APtr, SetCompare> time_set;

    std::srand(::time(nullptr));

    for (size_t i = 0; i < 10000; ++i) {
        auto a1 = std::make_shared<A>(std::rand());
        auto a2 = std::make_shared<A>(std::rand());
        auto a3 = std::make_shared<A>(std::rand());
        auto a4 = std::make_shared<A>(std::rand());
        auto a5 = std::make_shared<A>(std::rand());

        ASSERT_TRUE(time_set.emplace(a1).second);
        ASSERT_TRUE(time_set.emplace(a3).second);
        ASSERT_TRUE(time_set.emplace(a2).second);
        ASSERT_TRUE(time_set.emplace(a5).second);
        ASSERT_TRUE(time_set.emplace(a4).second);

        ASSERT_EQ(1, time_set.erase(a3));
        ASSERT_EQ(1, time_set.erase(a1));
        ASSERT_EQ(1, time_set.erase(a5));
        ASSERT_EQ(1, time_set.erase(a2));
        ASSERT_EQ(1, time_set.erase(a4));

        ASSERT_EQ(0, time_set.size());
    }
}

TEST(SET_ITER_TEST, ONLY_TEST) {
    std::set<std::string> strset;
    constexpr const size_t test_count = 10000;
    for (size_t i = 0; i < test_count; ++i) {
        strset.emplace(std::to_string(i));
    }
    ASSERT_EQ(test_count, strset.size());
    auto iter = strset.begin();
    while (iter != strset.end()) {
        std::string v = *iter;
        if (std::stoul(v) % 2 == 0) {
            iter = strset.erase(iter);
        } else {
            ++iter;
        }
        strset.emplace(v);
    }
    ASSERT_EQ(test_count, strset.size());
}

TEST(UNORDERED_MAP_EMPLACE_TEST, ONLY_TEST) {
    std::unordered_map<int, int> map;
    map.emplace(1, 1);
    map.emplace(1, 2);
    ASSERT_EQ(1, map[1]);

    map.insert(std::make_pair(1, 2));
    ASSERT_EQ(1, map[1]);

    map.at(1) = 2;
    ASSERT_EQ(2, map[1]);

    map[1] = 3;
    ASSERT_EQ(3, map[1]);
}

TEST(UNORDERED_MAP_TEST, ONLY_TEST) {
    std::unordered_map<std::string, int> map;
    for (int i = 0; i < 10; i++) {
        map.emplace(std::to_string(i), 0);
    }
    fprintf(stderr, "size: %lu\n", map.size());
    fprintf(stderr, "bucket count: %lu\n", map.bucket_count());
    for (size_t bucket = 0; bucket < map.bucket_count(); ++bucket) {
        fprintf(stderr, "bucket: %lu\n", bucket);
        for (auto iter = map.cbegin(bucket); iter != map.cend(bucket); ++iter) {
            fprintf(stderr, "\tkey: %s\n", iter->first.c_str());
        }
    }
    for (int i = 0; i < 30; ++i) {
        size_t bucket = ::random() % map.bucket_count();
        auto iter = map.cbegin(bucket);
        if (iter != map.cend(bucket)) {
            fprintf(stderr, "ranodm bucket: %lu, key: %s\n", bucket, iter->first.c_str());
        }
    }
}
