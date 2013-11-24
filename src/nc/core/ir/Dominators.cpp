/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include "Dominators.h"

#include <nc/common/Foreach.h>

#include "CFG.h"

namespace nc {
namespace core {
namespace ir {

Dominators::Dominators(const CFG &cfg) {
    /*
     * Each node dominates itself.
     */
    foreach (auto basicBlock, cfg.basicBlocks()) {
        dominators_[basicBlock].push_back(basicBlock);
    }

    /*
     * Recompute dominator sets until fixpoint.
     */
    bool changed;
    do {
        changed = false;

        foreach (auto basicBlock, cfg.basicBlocks()) {
            auto predecessors = cfg.getPredecessors(basicBlock);

            if (!predecessors.empty()) {
                boost::unordered_map<const BasicBlock *, std::size_t> intersection;
                foreach (auto predecessor, predecessors) {
                    foreach (auto predDominator, dominators_[predecessor]) {
                        ++intersection[predDominator];
                    }
                }

                std::vector<const BasicBlock *> newDominators;
                foreach (const auto &pair, intersection) {
                    if (pair.second == predecessors.size()) {
                        newDominators.push_back(pair.first);
                    }
                }
                newDominators.push_back(basicBlock);

                auto &oldDominators = dominators_[basicBlock];
                /* Sets grow monotonically, so we can just compare sizes. */
                if (newDominators.size() != oldDominators.size()) {
                    dominators_[basicBlock] = std::move(newDominators);
                    changed = true;
                }
            }
        }
    } while (changed);
}

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
