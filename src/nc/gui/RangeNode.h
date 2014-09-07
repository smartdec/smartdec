/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <algorithm>
#include <cassert>
#include <vector>

#include <nc/common/RangeClass.h>

namespace nc {

namespace core { namespace likec {
    class TreeNode;
}}

namespace gui {

class RangeNode {
    void *data_;
    int offset_;
    int size_;
    std::vector<RangeNode> children_;

public:
    RangeNode(void *data, int offset):
        data_(data), offset_(offset), size_(-1)
    {
        assert(offset >= 0);
    }

    void *data() const { return data_; }

    int offset() const { return offset_; }
    int size() const { assert(size_ >= 0); return size_; }
    void setSize(int size) { assert(size >= 0); size_ = size; }
    int endOffset() const { return offset() + size(); }

    Range<int> range() const { return Range<int>(offset(), endOffset()); }

    const std::vector<RangeNode> &children() const { return children_; }

    const RangeNode *getChildContaining(int offset) const {
        auto i = std::lower_bound(children_.begin(), children_.end(), offset,
                                  [](const RangeNode &node, int offset) { return node.endOffset() <= offset; });

        if (i == children_.end() || !i->range().contains(offset)) {
            return NULL;
        }

        return &*i;
    }

    RangeNode *addChild(RangeNode node) {
        assert(children_.empty() || children_.back().endOffset() <= node.offset());
        children_.push_back(std::move(node));
        return &children_.back();
    }
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
