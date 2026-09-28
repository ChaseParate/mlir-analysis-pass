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

using namespace mlir;

namespace mlir_analysis_pass::fpan {

void FPANAnalysis::setToEntryState(FPANLattice *lattice) {
  propagateIfChanged(lattice, lattice->join(FPANState::top()));
}

LogicalResult
FPANAnalysis::visitOperation(Operation *op,
                             ArrayRef<const FPANLattice *> operands,
                             ArrayRef<FPANLattice *> results) {
  // Raising a result to top says "this operation could produce anything",
  // which is always a sound answer and is what every unhandled case does.
  auto unknown = [&] {
    setAllToEntryStates(results);
    return success();
  };

  return unknown();
}

} // namespace mlir_analysis_pass::fpan
