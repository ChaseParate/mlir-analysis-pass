//===- FPANAnalysis.h - Sparse forward analysis over FPANState ------------===//

#ifndef FPAN_ANALYSIS_H
#define FPAN_ANALYSIS_H

#include "FPANDomain.h"
#include "mlir/Analysis/DataFlow/SparseAnalysis.h"

namespace mlir_analysis_pass::fpan {

using SELattice = mlir::dataflow::Lattice<SEState>;

class FPANAnalysis : public mlir::dataflow::SparseForwardDataFlowAnalysis<SELattice> {
public:
  using SparseForwardDataFlowAnalysis::SparseForwardDataFlowAnalysis;

  /// Transfer function: given the states of `op`'s operands, set the states of
  /// its results.  Must be monotone in the operand states.
  mlir::LogicalResult visitOperation(mlir::Operation *op,
                                     llvm::ArrayRef<const SELattice *> operands,
                                     llvm::ArrayRef<SELattice *> results) override;

  /// Calls are dispatched by SparseForwardDataFlowAnalysis separately from
  /// ordinary operations when interprocedural analysis is disabled.
  void visitExternalCall(mlir::CallOpInterface call, llvm::ArrayRef<const SELattice *> operands,
                         llvm::ArrayRef<SELattice *> results) override;

  /// The state of anything entering the analysis from outside: function
  /// arguments, and results the transfer function declines to reason about.
  void setToEntryState(SELattice *lattice) override;
};

} // namespace mlir_analysis_pass::fpan

#endif
