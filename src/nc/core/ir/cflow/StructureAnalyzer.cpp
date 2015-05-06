//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "StructureAnalyzer.h"

#include <algorithm>
#include <queue>

#include <boost/unordered_map.hpp>

#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>

#include <nc/core/ir/BasicBlock.h>
#include <nc/core/ir/Jump.h>
#include <nc/core/ir/dflow/Utils.h>
#include <nc/core/ir/misc/ArrayAccess.h>
#include <nc/core/ir/misc/BoundsCheck.h>
#include <nc/core/ir/misc/PatternRecognition.h>

#include "BasicNode.h"
#include "Dfs.h"
#include "Edge.h"
#include "Graph.h"
#include "LoopExplorer.h"
#include "Switch.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

void StructureAnalyzer::analyze() {
    analyze(graph_.root());
}

void StructureAnalyzer::analyze(Region *region) {
    bool changed;

    do {
        changed = false;

        /*
         * Classify edges, sort nodes topologically.
         */
        Dfs dfs(region);

        /*
         * Try to reduce various kinds of regions.
         */
        foreach (Node *node, dfs.postordering()) {
            if (reduceCompoundCondition(node)) {
                changed = true;
                break;
            }
        }
        if (changed) {
            continue;
        }

        foreach (Node *node, dfs.postordering()) {
            if (reduceCyclic(node, dfs)) {
                changed = true;
                break;
            }
        }
        if (changed) {
            continue;
        }

        foreach (Node *node, dfs.postordering()) {
            if (reduceBlock(node)) {
                changed = true;
                break;
            }
        }
        if (changed) {
            continue;
        }

        foreach (Node *node, dfs.postordering()) {
            if (reduceConditional(node)) {
                changed = true;
                break;
            }
        }
        if (changed) {
            continue;
        }

        foreach (Node *node, dfs.postordering()) {
            if (reduceSwitch(node) || reduceHopelessConditional(node)) {
                changed = true;
                break;
            }
        }
        if (changed) {
            continue;
        }
    } while (changed);
}

bool StructureAnalyzer::reduceBlock(Node *entry) {
    Node *uniquePredecessor = entry->uniquePredecessor();

    /*
     * Blocks are only needed for reducing ifs.
     * If the unique predecessor cannot be an if condition, stop.
     */
    if (!uniquePredecessor || !uniquePredecessor->isFork() || !uniquePredecessor->isCondition()) {
        return false;
    }

    /* Discover all the nodes in the block. */
    std::vector<Node *> traverse;
    Node *node = entry;
    do {
        traverse.push_back(node);
        node = node->uniqueSuccessor();
    } while (node && node->uniquePredecessor());

    /* Finally, create the region. */
    if (traverse.size() > 1) {
        Region *parent = entry->parent();
        Region *region = new Region(graph_, Region::BLOCK);
        foreach (Node *node, traverse) {
            region->addNode(node);
        }
        region->setEntry(entry);
        parent->addSubregion(region);
        return true;
    }

    return false;
}

bool StructureAnalyzer::reduceConditional(Node *entry) {
    if (!entry->isFork() || !entry->isCondition()) {
        return false;
    }

    Node *left = entry->outEdges()[0]->head();
    Node *right = entry->outEdges()[1]->head();

    Region *parent = entry->parent();

    /*
     * IF_THEN_ELSE
     */
    if (left->inEdges().size() == 1 && right->inEdges().size() == 1 &&
        left->outEdges().size() <= 1 && right->outEdges().size() <= 1 &&
        (left->outEdges().empty() || right->outEdges().empty() ||
         left->outEdges()[0]->head() == right->outEdges()[0]->head()))
    {
        Region *region = new Region(graph_, Region::IF_THEN_ELSE);
        region->addNode(entry);
        region->addNode(left);
        region->addNode(right);
        region->setEntry(entry);
        parent->addSubregion(region);
        return true;
    }

    /*
     * IF_THEN
     */
#define REDUCE(left, right)                                                         \
    if (left->inEdges().size() == 1 &&                                              \
        (left->outEdges().empty() ||                                                \
         (left->outEdges().size() == 1 && left->outEdges()[0]->head() == right)))   \
    {                                                                               \
        Region *region = new Region(graph_, Region::IF_THEN);                       \
        region->addNode(entry);                                                     \
        region->addNode(left);                                                      \
        region->setEntry(entry);                                                    \
        region->setExitBasicBlock(right->getEntryBasicBlock());                     \
        parent->addSubregion(region);                                               \
        return true;                                                                \
    }
    REDUCE(left, right)
    REDUCE(right, left)
#undef REDUCE

    return false;
}

bool StructureAnalyzer::reduceHopelessConditional(Node *entry) {
    if (!entry->isFork() || !entry->isCondition()) {
        return false;
    }

    Node *left = entry->outEdges()[0]->head();
    Node *right = entry->outEdges()[1]->head();

    Region *parent = entry->parent();

    Region *region = new Region(graph_, Region::IF_THEN_ELSE);
    region->addNode(entry);
    region->addNode(left);
    region->addNode(right);
    region->setEntry(entry);
    parent->addSubregion(region);

    return true;
}

bool StructureAnalyzer::reduceCompoundCondition(Node *entry) {
    if (!entry->isFork() || !entry->isCondition()) {
        return false;
    }

    Node *left = entry->outEdges()[0]->head();
    Node *right = entry->outEdges()[1]->head();

    Region *parent = entry->parent();

#define REDUCE(left, right)                                                                 \
    if (left->inEdges().size() == 1 && left->isFork() && left->isCondition() &&             \
        ((left->outEdges()[0]->head() == right && left->outEdges()[1]->head() != entry) ||  \
         (left->outEdges()[1]->head() == right && left->outEdges()[0]->head() != entry)))   \
    {                                                                                       \
        Region *region = new Region(graph_, Region::COMPOUND_CONDITION);                    \
        region->addNode(entry);                                                             \
        region->addNode(left);                                                              \
        region->setEntry(entry);                                                            \
        parent->addSubregion(region);                                                       \
        return true;                                                                        \
    }
    REDUCE(left, right)
    REDUCE(right, left)
#undef REDUCE

    return false;
}

bool StructureAnalyzer::reduceCyclic(Node *entry, const Dfs &dfs) {
    /*
     * Try to find the nodes constituting the loop.
     */
    LoopExplorer explorer(entry, dfs);

    if (explorer.loopNodes().empty()) {
        return false;
    }

    /*
     * Create the region.
     */
    Region *parent = entry->parent();
    Region *region = new Region(graph_, Region::LOOP);
    foreach (Node *n, explorer.loopNodes()) {
        region->addNode(n);
    }
    region->setEntry(entry);

    /*
     * Try to detect a WHILE loop: entry must be a condition and
     * must have an edge outside the region.
     *
     * We must do this early, before the edges are redirected by
     * Region::addSubregion() method.
     */
    if (entry->isFork() && entry->isCondition()) {
        foreach (const Edge *edge, entry->outEdges()) {
            if (edge->head()->parent() == parent) {
                region->setRegionKind(Region::WHILE);
                region->setExitBasicBlock(edge->head()->getEntryBasicBlock());
                break;
            }
        }
    }

    /*
     * Potential DO_WHILE conditions: pairs of the condition node
     * and respective loop exit basic block.
     */
    std::vector<std::pair<Node *, const BasicBlock *>> doWhileConditions;

    /*
     * Detect potential DO_WHILE conditions: one must be a condition,
     * have a (back) edge to the entry and an edge outside the region.
     *
     * We must do this early, before the edges are redirected by
     * Region::addSubregion() method.
     */
    foreach (Edge *edge, entry->inEdges()) {
        if (dfs.getEdgeType(edge) == Dfs::BACK) {
            Node *n = edge->tail();
            if (n->isFork() && n->isCondition()) {
                foreach (Edge *e, n->outEdges()) {
                    if (e->head()->parent() == parent) {
                        doWhileConditions.push_back(std::make_pair(n, e->head()->getEntryBasicBlock()));
                    }
                }
            }
        }
    }

    /*
     * Install this new region, redirect edges.
     */
    parent->addSubregion(region);

    /*
     * Remove 'continue' edges. They only make the structural
     * analysis in the loop region harder.
     */
    std::vector<Edge *> continueEdges = entry->inEdges();
    foreach (Edge *edge, continueEdges) {
        edge->setTail(0);
        edge->setHead(0);
    }

    /*
     * Run structural analysis inside the loop region.
     */
    analyze(region);

    /*
     * Try to detect a DO_WHILE loop.
     */
    foreach (const auto &pair, doWhileConditions) {
        if (pair.first->parent() == region) {
            region->setRegionKind(Region::DO_WHILE);
            region->setLoopCondition(pair.first);
            region->setExitBasicBlock(pair.second);
        }
    }

    return true;
}

bool StructureAnalyzer::reduceSwitch(Node *entry) {
    /*
     * Do not detect the same switch multiple times.
     */
    if (auto witch = entry->parent()->as<Switch>()) {
        if (witch->switchNode() == entry) {
            return false;
        }
    }

    /*
     * Entry must be a basic block node.
     */
    BasicNode *basicEntry = entry->as<BasicNode>();

    if (!basicEntry) {
        return false;
    }

    /*
     * Entry must end with an unconditional jump using a jump table,
     * and for which we can trace the jump index.
     */
    misc::ArrayAccess arrayAccess;
    std::size_t jumpTableSize;

    if (const Jump *jump = basicEntry->basicBlock()->getJump()) {
        if (jump->isUnconditional() && jump->thenTarget().table()) {
            arrayAccess = misc::recognizeArrayAccess(jump->thenTarget().address(), dataflow_);
            jumpTableSize = jump->thenTarget().table()->size();
        }
    }

    if (!arrayAccess) {
        return false;
    }

    /*
     * Typically, there is a bounds check before the switch:
     *
     * if (x < 10) {
     *     switch (x) {
     *       ...
     *     }
     * }
     */
    BasicNode *boundsCheckNode = NULL;

    if (Node *node = entry->uniquePredecessor()) {
        if (BasicNode *basicBlockNode = node->as<BasicNode>()) {
            if (const Jump *jump = basicBlockNode->basicBlock()->getJump()) {
                if (auto boundsCheck = misc::recognizeBoundsCheck(jump, entry->getEntryBasicBlock(), dataflow_)) {
                    if (dflow::getFirstCopy(boundsCheck.index(), dataflow_) ==
                        dflow::getFirstCopy(arrayAccess.index(), dataflow_))
                    {
                        boundsCheckNode = basicBlockNode;
                        jumpTableSize = std::min(jumpTableSize, static_cast<std::size_t>(boundsCheck.maxValue() + 1));
                    }
                }
            }
        }
    }

    /*
     * Node getting control if the bounds check fails is either an exit or a default.
     */
    Node *exitOrDefaultBranch = NULL;

    if (boundsCheckNode) {
        exitOrDefaultBranch = boundsCheckNode->getOtherSuccessor(entry);
    }

    /*
     * Direct successors of entry node and exitOrDefaultBranch are branches of the switch.
     */
    std::vector<Node *> branches;

    foreach (Edge *edge, entry->outEdges()) {
        branches.push_back(edge->head());
    }

    if (exitOrDefaultBranch) {
        branches.push_back(exitOrDefaultBranch);
    }

    std::sort(branches.begin(), branches.end());
    branches.erase(std::unique(branches.begin(), branches.end()), branches.end());

    /*
     * Using BFS, for each branch, compute the nodes reachable solely from this branch.
     */
    boost::unordered_map<Node *, Node *> node2branch;
    std::queue<Node *> queue;

    foreach (Node *node, branches) {
        queue.push(node);
        node2branch[node] = node;
    }

    while (!queue.empty()) {
        foreach (Edge *outEdge, queue.front()->outEdges()) {
            if (!nc::find(node2branch, outEdge->head())) {
                Node *branch = NULL;

                foreach (Edge *inEdge, outEdge->head()->inEdges()) {
                    if (Node *inBranch = nc::find(node2branch, inEdge->tail())) {
                        if (branch == NULL) {
                            branch = inBranch;
                        } else if (branch != inBranch) {
                            branch = NULL;
                            break;
                        }
                    } else {
                        branch = NULL;
                        break;
                    }
                }

                if (branch != NULL) {
                    node2branch[outEdge->head()] = branch;
                    queue.push(outEdge->head());
                }
            }
        }

        queue.pop();
    }

    /*
     * Returns how many branches actually join in the given node.
     */
    auto getJoinDegree = [&](const Node *node) -> std::size_t {
        std::vector<Node *> incomingBranches;

        foreach (const Edge *edge, node->inEdges()) {
            incomingBranches.push_back(find(node2branch, edge->tail()));
        }

        std::sort(incomingBranches.begin(), incomingBranches.end());
        incomingBranches.erase(std::unique(incomingBranches.begin(), incomingBranches.end()), incomingBranches.end());

        return incomingBranches.size();
    };

    /*
     * One of the branches can be actually an exit branch, i.e. go outside
     * the switch region.
     */
    Node *exitBranch = NULL;

    /* More than so many other branches must join in the exit branch. */
    std::size_t exitBranchJoinDegree = 2;

    foreach (Node *branch, branches) {
        auto mergeDegree = getJoinDegree(branch);
        if (mergeDegree > exitBranchJoinDegree) {
            exitBranchJoinDegree = mergeDegree;
            exitBranch = branch;
        }
    }

    /* If bounds check does not lead to an exit, it leads to the default branch. */
    Node *defaultBranch = NULL;

    if (exitBranch != exitOrDefaultBranch) {
        defaultBranch = exitOrDefaultBranch;
    }

    /*
     * Create the region.
     */
    Region *parent = entry->parent();
    Switch *region = new Switch(graph_, basicEntry, arrayAccess.index(), jumpTableSize);

    region->addNode(entry);

    if (boundsCheckNode) {
        region->addNode(boundsCheckNode);
        region->setBoundsCheckNode(boundsCheckNode);
        region->setEntry(boundsCheckNode);
    } else {
        region->setEntry(entry);
    }

    if (exitBranch) {
        region->setExitBasicBlock(exitBranch->getEntryBasicBlock());
    }

    if (defaultBranch) {
        region->setDefaultBasicBlock(defaultBranch->getEntryBasicBlock());
    }

    foreach (const auto &pair, node2branch) {
        assert(pair.second != NULL);

        if (pair.second != exitBranch) {
            region->addNode(pair.first);
        }
    }

    /*
     * Install this new region, redirect edges.
     */
    parent->addSubregion(region);

    return true;
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
