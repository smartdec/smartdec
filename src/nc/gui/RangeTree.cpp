/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "RangeTree.h"

#include <stack>

#include <nc/common/make_unique.h>
#include <nc/core/likec/Tree.h>

#include "RangeNode.h"

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
    while (auto child = node->getChildContaining(position)) {
        node = child;
        position -= node->offset();
    }
    return node;
}

namespace {

void getNodesInRange(const RangeNode &node, const Range<int> &range, std::vector<const RangeNode *> &result) {
    assert(node.range().overlaps(range.shifted(node.offset())));

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

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
