//===- ZeroAnalysis.h - Sparse forward analysis over ZeroState ------------===//

#ifndef ZERO_ANALYSIS_H
#define ZERO_ANALYSIS_H

#include "ZeroDomain.h"
#include "mlir/Analysis/DataFlow/SparseAnalysis.h"

namespace zero {

using ZeroLattice = mlir::dataflow::Lattice<ZeroState>;

class ZeroAnalysis
    : public mlir::dataflow::SparseForwardDataFlowAnalysis<ZeroLattice> {
public:
  using SparseForwardDataFlowAnalysis::SparseForwardDataFlowAnalysis;

  /// Transfer function: given the states of `op`'s operands, set the states of
  /// its results.  Must be monotone in the operand states.
  mlir::LogicalResult
  visitOperation(mlir::Operation *op,
                 llvm::ArrayRef<const ZeroLattice *> operands,
                 llvm::ArrayRef<ZeroLattice *> results) override;

  /// The state of anything entering the analysis from outside: function
  /// arguments, and results the transfer function declines to reason about.
  void setToEntryState(ZeroLattice *lattice) override;
};

} // namespace zero

#endif
