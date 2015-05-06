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

#include "Region.h"

#include <algorithm>
#include <cassert>

#include <QTextStream>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h> /* nc::contains() */
#include <nc/common/Unreachable.h>

#include "Edge.h"

namespace nc {
namespace core {
namespace ir {
namespace cflow {

void Region::setEntry(Node *entry) {
    assert(entry != NULL);
    assert(entry->parent() == this && "Entry must belong to the region.");

    entry_ = entry;
}

void Region::addNode(Node *node) {
    assert(node != NULL);

    node->setParent(this);
    nodes_.push_back(node);
}

void Region::addSubregion(Region *subregion) {
    assert(subregion != NULL);

    /* Remove subregion nodes from here. */
    nodes_.erase(std::remove_if(nodes_.begin(), nodes_.end(), [&](Node *n) { return n->parent() == subregion; }), nodes_.end());

    /* Add the subregion. */
    addNode(subregion);

    /*
     * Redirect edges properly.
     */

    std::vector<Edge *> edgesToSubregion;
    std::vector<Edge *> edgesFromSubregion;
    std::vector<Edge *> duplicateEdges;

    std::vector<Node *> tails;
    std::vector<Node *> heads;

    foreach (Node *node, subregion->nodes()) {
        foreach (Edge *edge, node->inEdges()) {
            assert(edge->tail()->parent() == this || edge->tail()->parent() == subregion);

            if (edge->tail()->parent() == this) {
                if (edge->head() == subregion->entry() && !contains(tails, edge->tail())) {
                    edgesToSubregion.push_back(edge);
                    tails.push_back(edge->tail());
                } else {
                    duplicateEdges.push_back(edge);
                }
            }
        }
        foreach (Edge *edge, node->outEdges()) {
            assert(edge->head()->parent() == this || edge->head()->parent() == subregion);

            if (edge->head()->parent() == this) {
                if (!contains(heads, edge->head())) {
                    edgesFromSubregion.push_back(edge);
                    heads.push_back(edge->head());
                } else {
                    duplicateEdges.push_back(edge);
                }
            }
        }
    }

    foreach (Edge *edge, edgesToSubregion) {
        edge->setHead(subregion);
    }
    foreach (Edge *edge, edgesFromSubregion) {
        edge->setTail(subregion);
    }
    foreach (Edge *edge, duplicateEdges) {
        edge->setTail(0);
        edge->setHead(0);
    }

    /* If the subregion contains our entry, this subregion is our new entry. */
    if (entry()->parent() == subregion) {
        assert(subregion->entry() == entry());
        setEntry(subregion);
    }
}

const BasicBlock *Region::getEntryBasicBlock() const {
    return entry()->getEntryBasicBlock();
}

bool Region::isCondition() const {
    return regionKind_ == COMPOUND_CONDITION;
}

void Region::print(QTextStream &out) const {
    out << "node" << this << " [shape=box,label=\"";

    switch (regionKind_) {
        case UNKNOWN:
            out << "UNKNOWN";
            break;
        case BLOCK:
            out << "BLOCK";
            break;
        case IF_THEN:
            out << "IF_THEN";
            break;
        case IF_THEN_ELSE:
            out << "IF_THEN_ELSE";
            break;
        case COMPOUND_CONDITION:
            out << "COMPOUND_CONDITION";
            break;
        case LOOP:
            out << "LOOP";
            break;
        case DO_WHILE:
            out << "DO_WHILE";
            break;
        case WHILE:
            out << "WHILE";
            break;
        default:
            unreachable();
    }

    out << "\"]" << endl;

    out << "subgraph cluster" << this << " {" << endl;
    foreach (const Node *node, nodes()) {
        out << *node;
    }
    out << '}' << endl;

    out << "node" << this << " -> node" << (entry() ? entry() : nodes()[0])
        << " [color=\"blue\" lhead=\"cluster" << this << "\"]" << endl;
}

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
