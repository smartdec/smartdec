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

#include "InstructionAnalyzer.h"

#include <algorithm> /* For std::max. */

#include <nc/common/Unreachable.h>
#include <nc/common/make_unique.h>

#include <nc/core/arch/Instruction.h>
#include <nc/core/arch/Operands.h>
#include <nc/core/ir/Program.h>
#include <nc/core/ir/Statement.h>
#include <nc/core/ir/Terms.h>

#include "InvalidInstructionException.h"

namespace nc {
namespace core {
namespace arch {
namespace irgen {

std::unique_ptr<ir::Term> InstructionAnalyzer::createTerm(const Operand *operand) const {
    assert(operand != NULL);

    return doCreateTerm(operand);
}

void InstructionAnalyzer::createStatements(const Instruction *instruction, ir::Program *program) const {
    assert(instruction);

    try {
        doCreateStatements(instruction, program);
    } catch (nc::Exception &e) {
        if (!boost::get_error_info<ExceptionInstruction>(e)) {
            e << ExceptionInstruction(instruction);
        }
        throw;
    }
}

void InstructionAnalyzer::doCreateStatements(const Instruction * /*instruction*/, ir::Program * /*program*/) const {
    return;
}

std::unique_ptr<ir::Term> InstructionAnalyzer::doCreateTerm(const Operand *operand) const {
    switch(operand->kind()) {
    case Operand::REGISTER: {
        return std::make_unique<ir::MemoryLocationAccess>(operand->asRegister()->memoryLocation());
    }
    case Operand::ADDITION: {
        const AdditionOperand *addition = operand->asAddition();

        auto left = createTerm(addition->left());
        auto right = createTerm(addition->right());

        SmallBitSize size = std::max(left->size(), right->size());

        /* [ebx + 0xfd] --- 0xfd must be sign-extended. */
        if (left->size() < size) {
            left = std::make_unique<ir::UnaryOperator>(ir::UnaryOperator::SIGN_EXTEND, std::move(left), size);
        }
        if (right->size() < size) {
            right = std::make_unique<ir::UnaryOperator>(ir::UnaryOperator::SIGN_EXTEND, std::move(right), size);
        }

        return std::make_unique<ir::BinaryOperator>(ir::BinaryOperator::ADD, std::move(left), std::move(right));
    }
    case Operand::MULTIPLICATION: {
        const MultiplicationOperand *multiplication = operand->asMultiplication();

        auto left = createTerm(multiplication->left());
        auto right = createTerm(multiplication->right());

        SmallBitSize size = std::max(left->size(), right->size());

        /* [ebx + esi * 4] --- 4 must be zero-extended. */
        if (left->size() < size) {
            left = std::make_unique<ir::UnaryOperator>(ir::UnaryOperator::ZERO_EXTEND, std::move(left), size);
        }
        if (right->size() < size) {
            right = std::make_unique<ir::UnaryOperator>(ir::UnaryOperator::ZERO_EXTEND, std::move(right), size);
        }

        return std::make_unique<ir::BinaryOperator>(ir::BinaryOperator::MUL, std::move(left), std::move(right));
    }
    case Operand::DEREFERENCE: {
        const DereferenceOperand *dereference = operand->asDereference();

        return std::make_unique<ir::Dereference>(
            createTerm(dereference->operand()), 
            ir::MemoryDomain::MEMORY, 
            dereference->size()
        );
    }
    case Operand::BIT_RANGE: {
        const BitRangeOperand *bitRange = operand->asBitRange();

        SmallBitSize operandSize = bitRange->operand()->size();

        std::unique_ptr<ir::Term> result(new ir::BinaryOperator(
            ir::BinaryOperator::BITWISE_AND,
            std::make_unique<ir::BinaryOperator>(
                ir::BinaryOperator::SHR,
                createTerm(bitRange->operand()),
                std::make_unique<ir::Constant>(SizedValue(bitRange->offset(), operandSize))
            ),
            std::make_unique<ir::Constant>(SizedValue((static_cast<ConstantValue>(1) << bitRange->size()) - 1, operandSize)),
            bitRange->size()
        ));

        return result;
    }
    case Operand::CONSTANT: {
        const ConstantOperand *constant = operand->asConstant();

        return std::make_unique<ir::Constant>(constant->value());
    }
    default:
        unreachable();
    }
}

} // namespace irgen
} // namespace arch
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
