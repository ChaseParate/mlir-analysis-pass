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

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Matchers.h"

using namespace mlir;

namespace mlir_analysis_pass::fpan {

void FPANAnalysis::setToEntryState(SELattice *lattice) {
  propagateIfChanged(lattice, lattice->join(SEState::top()));
}

LogicalResult FPANAnalysis::visitOperation(Operation *op, ArrayRef<const SELattice *> operands,
                                           ArrayRef<SELattice *> results) {
  auto unknown = [&] {
    setAllToEntryStates(results);
    return success();
  };

  if (op->getNumResults() != 1 || !op->getResult(0).getType().isF32())
    return unknown();
  SELattice *result = results[0];

  // Float Constants
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

} // namespace mlir_analysis_pass::fpan
