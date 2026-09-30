//===- FPANAnalysis.cpp - Transfer functions ------------------------------===//
//
// The transfer function: given what is known about an operation's operands,
// state what is known about its results.  This file and FPANDomain.h are the
// two to replace when building a different analysis; the rest of the project
// is scaffolding.
//
// This intentionally has no rules yet. Add the FPAN domain and transfer rules
// here as they are defined.
//
//===----------------------------------------------------------------------===//

#include "FPANAnalysis.h"

#include <optional>
#include <utility>

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Matchers.h"

using namespace mlir;

namespace mlir_analysis_pass::fpan {

namespace twosum {

static bool isTwoSumCall(Operation *op) {
  auto call = dyn_cast<func::CallOp>(op);
  return call && call.getCallee() == "__fpan_twosum_f32" &&
         (call.getNumOperands() == 2 &&
          llvm::all_of(call.getOperandTypes(), [](Type type) { return type.isF32(); })) &&
         (call.getNumResults() == 2 &&
          llvm::all_of(call.getResultTypes(), [](Type type) { return type.isF32(); }));
}

using TwoSumLemmaReturn = std::optional<std::pair<SEState, SEState>>;
using TwoSumLemma = TwoSumLemmaReturn (*)(const SEState &lhs, const SEState &rhs);

static TwoSumLemmaReturn applyLemmaZ1(const SEState &lhs, const SEState &rhs) {
  const bool leftPos = lhs.isOnlyPositiveZero(), leftNeg = lhs.isOnlyNegativeZero();
  const bool rightPos = rhs.isOnlyPositiveZero(), rightNeg = rhs.isOnlyNegativeZero();

  if (leftPos && rightPos) {
    return std::make_pair(SEState::positiveZero(), SEState::positiveZero());
  } else if ((leftPos && rightNeg) || (leftNeg && rightPos)) {
    return std::make_pair(SEState::positiveZero(), SEState::positiveZero());
  } else if (leftNeg && rightNeg) {
    return std::make_pair(SEState::negativeZero(), SEState::positiveZero());
  }

  return std::nullopt;
}

static TwoSumLemmaReturn applyLemmaZ2(const SEState &lhs, const SEState &rhs) {
  if (lhs.isNotZero() && rhs.isOnlyZero()) {
    return std::make_pair(lhs, SEState::positiveZero());
  } else if (rhs.isNotZero() && lhs.isOnlyZero()) {
    return std::make_pair(rhs, SEState::positiveZero());
  }

  return std::nullopt;
}

static TwoSumLemmaReturn applyLemmaSEI(const SEState &lhs, const SEState &rhs) {
  constexpr std::uint8_t maximumExponentDifference = kPrecision + 1;

  if (lhs.isNotZero() && rhs.isNotZero() &&
      lhs.forAllSetExponentsAndSigns([&](Exponent lhsExponent, bool lhsNegative) {
        return rhs.forAllSetExponentsAndSigns([&](Exponent rhsExponent, bool rhsNegative) {
          const int exponentDifference = std::abs(lhsExponent - rhsExponent);
          return (exponentDifference < maximumExponentDifference ||
                  (exponentDifference == maximumExponentDifference && lhsNegative == rhsNegative));
        });
      }))
    return std::make_pair(lhs, rhs);

  return std::nullopt;
}

static constexpr TwoSumLemma kLemmas[] = {applyLemmaZ1, applyLemmaZ2, applyLemmaSEI};

} // namespace twosum

LogicalResult FPANAnalysis::visitOperation(Operation *op, ArrayRef<const SELattice *> operands,
                                           ArrayRef<SELattice *> results) {
  auto unknown = [&] {
    setAllToEntryStates(results);
    return success();
  };

  if (op->getNumResults() != 1 || !op->getResult(0).getType().isF32())
    return unknown();
  SELattice *result = results[0];

  FloatAttr attr;
  if (matchPattern(op, m_Constant(&attr))) {
    const llvm::APFloat value = attr.getValue();
    const std::optional<SEState> state = SEState::fromFloat(value);

    if (!state.has_value())
      return unknown();

    propagateIfChanged(result, result->join(*state));
    return success();
  }

  return unknown();
}

void FPANAnalysis::visitExternalCall(CallOpInterface call, ArrayRef<const SELattice *> operands,
                                     ArrayRef<SELattice *> results) {
  if (twosum::isTwoSumCall(call.getOperation())) {
    const SEState &lhs = operands[0]->getValue(), &rhs = operands[1]->getValue();

    twosum::TwoSumLemmaReturn res;
    for (const auto &lemma : twosum::kLemmas) {
      if ((res = lemma(lhs, rhs))) {
        propagateIfChanged(results[0], results[0]->join(res->first));
        propagateIfChanged(results[1], results[1]->join(res->second));
        return;
      }
    }
  }

  setAllToEntryStates(results);
}

void FPANAnalysis::setToEntryState(SELattice *lattice) {
  propagateIfChanged(lattice, lattice->join(SEState::top()));
}

} // namespace mlir_analysis_pass::fpan
