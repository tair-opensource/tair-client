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

#include <cstdint>
#include <deque>

#include "common/Noncopyable.hpp"

namespace tair::common {

class CidrTrieTree : private Noncopyable {
public:
    CidrTrieTree() = default;
    ~CidrTrieTree();

    // Puts the given network address to prefix tree
    // @param ip IP of subnet
    // @param cidr subnet length
    // @param as value
    void trieNodePut(const char *ip, int cidr, uint64_t as);

    // Searches for network for the given IP address
    // @param ip IP of subnet
    // @param cidr subnet length
    // @param as store value to
    bool trieNodeSearch(const char *ip, int cidr, uint64_t *as) const;

    bool empty() const;

    void clear();

private:
    // Node of tree
    struct Node {
        Node *left = nullptr;
        Node *right = nullptr;
        uint64_t as = 0;    // Network ID (value)
        bool as_in = false; // Indicator: is as loaded? (not empty)
    };

    // Memory stack for nodes
    struct TrieMemory {
        bool isFull() const {
            return index_ + 1 == TRIE_MEMORY_CELL_SIZE;
        }

        Node *allocNode() {
            if (!isFull()) {
                return &cells_[index_++];
            }
            return nullptr;
        }

    private:
        constexpr static const int TRIE_MEMORY_CELL_SIZE = 32768;

    private:
        int index_ = 0;
        Node cells_[TRIE_MEMORY_CELL_SIZE]; // Memory cells
    };

    Node *createNode();

private:
    Node *root_ = nullptr;
    std::deque<TrieMemory *> memory_cells_;
};

} // namespace tair::common
