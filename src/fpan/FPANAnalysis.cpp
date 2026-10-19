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

static const char *kTwoSumFunctionName = "__fpan_twosum_f32";

static bool isTwoSumCallee(Operation *op) {
  if (auto call = dyn_cast<func::CallOp>(op))
    return call.getCallee() == kTwoSumFunctionName;

  // `mlir-translate --import-llvm` represents an LLVM IR call with `llvm.call`, rather than
  // `func.call`.
  if (auto call = dyn_cast<LLVM::CallOp>(op)) {
    const std::optional<llvm::StringRef> callee = call.getCallee();
    return callee && *callee == kTwoSumFunctionName;
  }

  return false;
}

static bool isTwoSumFuncCall(Operation *op) {
  return isa<func::CallOp>(op) && isTwoSumCallee(op) && op->getNumOperands() == 2 &&
         llvm::all_of(op->getOperandTypes(), [](Type type) { return type.isF32(); }) &&
         op->getNumResults() == 2 &&
         llvm::all_of(op->getResultTypes(), [](Type type) { return type.isF32(); });
}

static bool isTwoSumAggregateCall(Operation *op) {
  auto call = dyn_cast<LLVM::CallOp>(op);
  if (!call || !isTwoSumCallee(op) || op->getNumOperands() != 2 ||
      !llvm::all_of(op->getOperandTypes(), [](Type type) { return type.isF32(); }) ||
      op->getNumResults() != 1)
    return false;

  auto resultType = dyn_cast<LLVM::LLVMStructType>(op->getResult(0).getType());
  return resultType && resultType.getBody().size() == 2 &&
         llvm::all_of(resultType.getBody(), [](Type type) { return type.isF32(); });
}

static bool isRecognizedTwoSumExtract(LLVM::ExtractValueOp extract) {
  auto call = extract.getContainer().getDefiningOp<LLVM::CallOp>();
  if (!call || !isTwoSumAggregateCall(call))
    return false;

  ArrayRef<int64_t> position = extract.getPosition();
  return position.size() == 1 && (position[0] == 0 || position[0] == 1);
}

using TwoSumLemmaReturn = std::optional<std::pair<SEState, SEState>>;
using TwoSumLemma = TwoSumLemmaReturn (*)(const SEState &x, const SEState &y);

static TwoSumLemmaReturn applyLemmaZ1(const SEState &x, const SEState &y) {
  const bool xPos = x.isOnlyPositiveZero(), xNeg = x.isOnlyNegativeZero();
  const bool yPos = y.isOnlyPositiveZero(), yNeg = y.isOnlyNegativeZero();

  if (xPos && yPos) {
    return std::make_pair(SEState::positiveZero(), SEState::positiveZero());
  } else if ((xPos && yNeg) || (xNeg && yPos)) {
    return std::make_pair(SEState::positiveZero(), SEState::positiveZero());
  } else if (xNeg && yNeg) {
    return std::make_pair(SEState::negativeZero(), SEState::positiveZero());
  }

  return std::nullopt;
}

static TwoSumLemmaReturn applyLemmaZ2(const SEState &x, const SEState &y) {
  if (!x.isZero() && y.isOnlyZero()) {
    return std::make_pair(x, SEState::positiveZero());
  } else if (!y.isZero() && x.isOnlyZero()) {
    return std::make_pair(y, SEState::positiveZero());
  }

  return std::nullopt;
}

static TwoSumLemmaReturn applyLemmaI(const SEState &x, const SEState &y) {
  const auto lemma = [](const SEState &x, const SEState y) -> bool {
    return x.forAllSetExponentsAndSigns([&](Exponent ex, bool sx) {
      return y.forAllSetExponentsAndSigns([&](Exponent ey, bool sy) {
        return ex > ey + kPrecision + 1 || (ex == ey + kPrecision + 1 && sx == sy);
      });
    });
  };

  if (!x.isZero() && !y.isZero() && (lemma(x, y) || lemma(y, x)))
    return std::make_pair(x, y);

  return std::nullopt;
}

static TwoSumLemmaReturn applyLemmaS1(const SEState &x, const SEState &y) {
  const auto applies = [](const SEState &large, const SEState &small) {
    return !large.isZero() && !small.isZero() &&
           large.forAllSetExponentsAndSigns([&](Exponent el, bool sl) {
             return small.forAllSetExponentsAndSigns([&](Exponent es, bool ss) {
               // Also avoid exponent overflow (see statement 1) or reaching subnormal.
               return sl == ss && el == es + kPrecision && el < kEMax &&
                      es - (kPrecision - 1) >= kEMin;
             });
           });
  };

  const SEState *large = nullptr;
  const SEState *small = nullptr;
  if (applies(x, y)) {
    large = &x;
    small = &y;
  } else if (applies(y, x)) {
    large = &y;
    small = &x;
  } else {
    return std::nullopt;
  }

  // Statement 2
  SEState sum = *large;
  SEState error = *small;

  // Statement 1
  large->forEachSetExponentAndSign([&](Exponent ex, bool sx) {
    // ex <= es <= ex + 1
    sum.set(ex + 1, sx); // (ex, sx) was already set by Statement 2.

    // se != sy and ey - (p - 1) <= ee <= ex - p.
    // Since ey = ex - p and sx == sy: ex - 2p + 1 <= ee <= ex - p, and se != sx.
    for (int ee = ex - 2 * kPrecision + 1; ee <= ex - kPrecision; ++ee)
      error.set(static_cast<Exponent>(ee), !sx);
  });

  return std::make_pair(sum, error);
}

static constexpr TwoSumLemma kLemmas[] = {applyLemmaZ1, applyLemmaZ2, applyLemmaI, applyLemmaS1};

} // namespace twosum

LogicalResult FPANAnalysis::visitOperation(Operation *op, ArrayRef<const SELattice *> operands,
                                           ArrayRef<SELattice *> results) {
  auto unknown = [&] {
    setAllToEntryStates(results);
    return success();
  };

  if (auto extract = dyn_cast<LLVM::ExtractValueOp>(op);
      extract && twosum::isRecognizedTwoSumExtract(extract))
    // The aggregate TwoSum call writes this scalar result's state directly.
    return success();

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
  if (twosum::isTwoSumFuncCall(call.getOperation())) {
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

  if (twosum::isTwoSumAggregateCall(call.getOperation())) {
    const SEState &lhs = operands[0]->getValue(), &rhs = operands[1]->getValue();
    twosum::TwoSumLemmaReturn lemmaResult;
    for (const auto &lemma : twosum::kLemmas) {
      if ((lemmaResult = lemma(lhs, rhs)))
        break;
    }

    for (Operation *user : call->getResult(0).getUsers()) {
      auto extract = dyn_cast<LLVM::ExtractValueOp>(user);
      if (!extract || !twosum::isRecognizedTwoSumExtract(extract))
        continue;

      SELattice *extractResult = getOrCreate<SELattice>(extract.getResult());
      if (!lemmaResult) {
        setToEntryState(extractResult);
        continue;
      }

      const int64_t position = extract.getPosition()[0];
      propagateIfChanged(extractResult, extractResult->join(position == 0 ? lemmaResult->first
                                                                          : lemmaResult->second));
    }

    // SEState is scalar-only: the aggregate result carries no FPAN fact.
    setAllToEntryStates(results);
    return;
  }

  setAllToEntryStates(results);
}

void FPANAnalysis::setToEntryState(SELattice *lattice) {
  propagateIfChanged(lattice, lattice->join(SEState::top()));
}

} // namespace mlir_analysis_pass::fpan
