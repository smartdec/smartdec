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

#include "Graph.h"

#include <QTextStream>

#include <nc/common/Foreach.h>

#include "Edge.h"
#include "Node.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

Graph::~Graph() {
    foreach (Node *node, nodes_) {
        delete node;
    }
    foreach (Edge *edge, edges_) {
        delete edge;
    }
}

void Graph::addNode(Node *node) {
    nodes_.push_back(node);
}

Edge *Graph::createEdge(Node *tail, Node *head) {
    Edge *edge = new Edge(tail, head);
    edges_.push_back(edge);
    return edge;
}

void Graph::print(QTextStream &out) const {
    out << *root();
    foreach (const Edge *edge, edges_) {
        if (edge->tail() && edge->head()) {
            out << "node" << edge->tail() << " -> node" << edge->head() << endl;
        }
    }
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
