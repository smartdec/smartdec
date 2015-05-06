/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <cassert>

#include "Node.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

class Switch;

/**
 * Region is a set of nodes with a single entry and possibly multiple exits.
 */
class Region: public Node {
    NC_CLASS_WITH_KINDS(Region, regionKind)

    public:

    /**
     * Region kind.
     */
    enum RegionKind {
        UNKNOWN, ///< Unknown region.
        BLOCK, ///< Sequence of consecutive nodes.
        COMPOUND_CONDITION, ///< Short-circuit computation of && or ||.
        IF_THEN, ///< If-then.
        IF_THEN_ELSE, ///< If-then-else.
        LOOP, ///< General loop.
        WHILE, ///< Loop with precondition.
        DO_WHILE, ///< Loop with postcondition.
        SWITCH ///< Switch.
    };

    private:

    Node *entry_; ///< Region's entry node.
    std::vector<Node *> nodes_; ///< Nodes of the region.
    const BasicBlock *exitBasicBlock_; ///< Exit basic block.
    Node *loopCondition_; ///< Loop condition.

    public:

    /**
     * Class constructor.
     *
     * \param graph         Graph this node belongs to.
     * \param regionKind    Region kind.
     */
    Region(Graph &graph, RegionKind regionKind):
        Node(graph, REGION), regionKind_(regionKind), entry_(NULL), exitBasicBlock_(NULL), loopCondition_(NULL)
    {}

    /**
     * Sets region kind.
     *
     * \param[in] regionKind Region kind.
     */
    void setRegionKind(RegionKind regionKind) { regionKind_ = regionKind; }

    /**
     * \return Pointer to the region's entry. Can be NULL.
     */
    Node *entry() const { return entry_; }

    /**
     * Sets region entry.
     *
     * \param[in] entry Valid pointer to the new region entry.
     */
    void setEntry(Node *entry);

    /**
     * \return Region nodes.
     */
    const std::vector<Node *> &nodes() const { return nodes_; }

    /**
     * Adds node to the region.
     *
     * \param[in] node Valid pointer to a node.
     */
    void addNode(Node *node);

    /**
     * Adds subregion to the region.
     * All nodes of the subregion are removed from the region.
     * All edges from the nodes of the subregion become edges from the subregion.
     * All edges to the entry node of the subregion become edges to the subregion.
     * All other edges as well as duplicate edges are deleted.
     *
     * \param[in] subregion Valid pointer to the subregion.
     */
    void addSubregion(Region *subregion);

    /**
     * \return Pointer to the exit basic block. Can be NULL.
     */
    const BasicBlock *exitBasicBlock() const { return exitBasicBlock_; }

    /**
     * Sets the exit basic block.
     * Exit basic block gets control when the region finishes execution.
     *
     * \param basicBlock Pointer to the new exit basic block. Can be NULL.
     */
    void setExitBasicBlock(const BasicBlock *basicBlock) { exitBasicBlock_ = basicBlock; }

    /**
     * \return Pointer to the node being the condition of the do-while loop.
     * Can be NULL iff region is not of DO_WHILE type.
     */
    Node *loopCondition() const { assert(regionKind() == DO_WHILE); return loopCondition_; }

    /**
     * Sets the node being the condition of the do-while loop.
     *
     * \param condition Pointer to the new do-while loop condition.
     */
    void setLoopCondition(Node *condition) { assert(regionKind() == DO_WHILE); loopCondition_ = condition; }

    virtual const BasicBlock *getEntryBasicBlock() const override;

    virtual bool isCondition() const override;

    virtual void print(QTextStream &out) const override;
};

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

NC_REGISTER_CLASS_KIND(nc::core::ir::cflow::Region, nc::core::ir::cflow::Switch, nc::core::ir::cflow::Region::SWITCH)

/* vim:set et sts=4 sw=4: */
