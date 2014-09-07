/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <memory>
#include <vector>

#include "RangeNode.h"

namespace nc {

namespace core { namespace likec {
    class TreeNode;
    class Tree;
}}

namespace gui {

class RangeTree {
    std::unique_ptr<RangeNode> root_;

public:
    RangeTree();
    ~RangeTree();

    void setRoot(std::unique_ptr<RangeNode> root);
    const RangeNode *getLeafAt(int position) const;
    std::vector<const RangeNode *> getNodesIn(const Range<int> &range) const;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
