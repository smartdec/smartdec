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

#include <boost/noncopyable.hpp>

#include <nc/common/Printable.h>

#include "Region.h"

namespace nc {
namespace core {
namespace ir {

class BasicBlock;

namespace cflow {

class BasicBlockNode;
class Edge;
class Node;

/**
 * Control flow graph data structure suitable for performing structural analysis on it.
 */
class Graph: public PrintableBase<Graph>, boost::noncopyable {
    std::vector<Node *> nodes_; ///< All nodes of the graph.
    std::vector<Edge *> edges_; ///< All edges of the graph.
    Region *root_; ///< Root region.

    public:

    /**
     * Class constructor.
     */
    Graph(): root_(NULL) {}

    /**
     * Class destructor.
     */
    ~Graph();

    /**
     * Sets the root region.
     *
     * \param[in] root Pointer to the new root region. Can be NULL.
     */
    void setRoot(Region *root) { root_ = root; }

    /**
     * \return Pointer to the root region. Can be NULL.
     */
    Region *root() { return root_; }

    /**
     * \return Pointer to the root region. Can be NULL.
     */
    const Region *root() const { return root_; }

    /**
     * \return All nodes of the graph.
     */
    const std::vector<Node *> &nodes() const { return nodes_; }

    /**
     * \return All edges of the graph.
     */
    const std::vector<Edge *> &edges() const { return edges_; }

    /**
     * Adds a node into the graph.
     *
     * \param node Valid pointer to a node.
     *
     * \return Given pointer to the node.
     */
    void addNode(Node *node);

    /**
     * Creates new edge in the graph.
     *
     * \param[in] tail The node edge goes from.
     * \param[in] head The node edge goes to.
     *
     * \return New edge.
     */
    Edge *createEdge(Node *tail, Node *head);

    /**
     * Prints the root region to given stream in DOT language.
     *
     * \param out Output stream.
     */
    void print(QTextStream &out) const;
};

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
