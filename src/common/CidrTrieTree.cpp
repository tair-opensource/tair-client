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
#include "common/CidrTrieTree.hpp"

#include "common/Logger.hpp"

namespace tair::common {

CidrTrieTree::~CidrTrieTree() {
    clear();
}

CidrTrieTree::Node *CidrTrieTree::createNode() {
    TrieMemory *current_cell = memory_cells_.empty() ? nullptr : memory_cells_.back();
    // add new memory cell
    if (!current_cell || current_cell->isFull()) {
        TrieMemory *cells = new TrieMemory();
        memory_cells_.emplace_back(cells);
        current_cell = cells;
    }
    return current_cell->allocNode();
}

void CidrTrieTree::trieNodePut(const char *ip, int cidr, uint64_t as) {
    if (!root_) {
        root_ = createNode();
    }
    Node *current = root_;
    // add each bit of address prefix to tree
    for (int i = 0; i < cidr; i++) {
        // bit is 1 => go right
        if (((ip[i / 8] >> (7 - i % 8)) & 1) > 0) {
            // create path if not exists
            if (!current->right) {
                current->right = createNode();
            }
            // next
            current = current->right;
        } else { // bit is 0 => go left
            // create path if not exists
            if (!current->left) {
                current->left = createNode();
            }
            // next
            current = current->left;
        }
        // last bit => add value to node
        if (i + 1 == cidr) {
            if (!current->as_in) {
                current->as_in = true;
                current->as = as;
            } else if (current->as != as) {
                LOG_WARN("Some network collides: {} and {}", current->as, as);
            }
        }
    }
}

bool CidrTrieTree::trieNodeSearch(const char *ip, int cidr, uint64_t *as) const {
    if (!root_) {
        return false;
    }
    Node *current = root_;
    // means 0.0.0.0/0
    if (!current->left && !current->right) {
        return true;
    }
    for (int i = 0; i < cidr; i++) {
        for (int u = 7; u >= 0; u--) {
            // search right
            if (((ip[i] >> u) & 1) > 0) {
                current = current->right;
            } else { // search left
                current = current->left;
            }
            // end of road
            if (!current) {
                return false;
            }
            if (current->as_in) {
                // finally get you, just return asap
                if (as) {
                    *as = current->as;
                }
                return true;
            }
        }
    }
    return false;
}

bool CidrTrieTree::empty() const {
    return !root_;
}

void CidrTrieTree::clear() {
    for (auto &cell : memory_cells_) {
        delete cell;
    }
    memory_cells_.clear();
    root_ = nullptr;
}

} // namespace tair::common
