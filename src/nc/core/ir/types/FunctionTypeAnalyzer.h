/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

namespace nc {
namespace core {
namespace ir {
namespace types {

/**
 * This class computes 
 */
class FunctionTypeAnalyzer {
    Types &types_; ///< Information about terms' types.

    public:

    FunctionTypeAnalyzer(Types &types, const Function *function, const liveness::Liveness &liveness);
};

}}}} // namespace nc::core::ir::types

/* vim:set et sts=4 sw=4: */
