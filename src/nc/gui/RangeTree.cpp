/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "RangeTree.h"

#include <stack>

#include <nc/common/make_unique.h>
#include <nc/core/likec/Tree.h>

namespace nc {
namespace gui {

RangeTree::RangeTree() {}

RangeTree::~RangeTree() {}

void RangeTree::setRoot(std::unique_ptr<RangeNode> root) {
    assert(!root || root->offset() == 0);
    root_ = std::move(root);
}

const RangeNode *RangeTree::getLeafAt(int position) const {
    if (!root_ || !root_->range().contains(position)) {
        return NULL;
    }

    const RangeNode *node = root_.get();

    while (true) {
        auto i = std::lower_bound(node->children().begin(), node->children().end(), position,
                                  [](const RangeNode &node, int offset) { return node.endOffset() <= offset; });

        if (i == node->children().end() || !i->range().contains(position)) {
            return node;
        }

        node = &*i;
        position -= node->offset();
    }
}

namespace {

void getNodesInRange(const RangeNode &node, const Range<int> &range, std::vector<const RangeNode *> &result) {
    result.push_back(&node);

    auto i = std::lower_bound(node.children().begin(), node.children().end(), range.start(),
                              [](const RangeNode &node, int offset) { return node.endOffset() <= offset; });
    auto iend = node.children().end();

    for (; i != iend && i->offset() < range.end(); ++i) {
        getNodesInRange(*i, range.shifted(-i->offset()), result);
    }
}

} // anonymous namespace

std::vector<const RangeNode *> RangeTree::getNodesIn(const Range<int> &range) const {
    std::vector<const RangeNode *> result;
    if (root_) {
        if (root_->range().overlaps(range)) {
            getNodesInRange(*root_, range, result);
        }
    }
    return result;
}

Range<int> RangeTree::getRange(const RangeNode *node) const {
    assert(node != NULL);
    assert(root_ != NULL);

    if (node->parent() == NULL && node != root_.get()) {
        root_->updateParentPointers();
        assert(node->parent() != NULL);
    }

    int offset = 0;
    for (auto current = node; current != root_.get(); current = current->parent()) {
        offset += current->offset();
    }
    return make_range(offset, offset + node->size());
}

namespace {

void doHandleInsertion(RangeNode &node, int position, int nchars) {
    node.setSize(node.size() + nchars);

    auto i = std::lower_bound(node.children().begin(), node.children().end(), position,
                              [](const RangeNode &node, int offset) { return node.endOffset() <= offset; });

    if (i != node.children().end()) {
        if (i->range().contains(position)) {
            doHandleInsertion(*i, position - i->offset(), nchars);
            ++i;
        }

        for (auto iend = node.children().end(); i != iend; ++i) {
            i->setOffset(i->offset() + nchars);
        }
    }
}

} // anonymous namespace

void RangeTree::handleInsertion(int position, int nchars) {
    if (root_ && root_->range().contains(position)) {
        doHandleInsertion(*root_, position, nchars);
    }
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
