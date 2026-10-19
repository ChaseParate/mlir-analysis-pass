//===- FPANPlugin.cpp - Pass definition and plugin entry point -------------===//
//
// Scaffolding: wires the analysis into a pass and exposes it to mlir-opt.
//
//===----------------------------------------------------------------------===//

#include <array>
#include <cstdint>

#include "Annotate.h"
#include "FPANAnalysis.h"

#include "mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"
#include "mlir/Analysis/DataFlow/DeadCodeAnalysis.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {

struct FPANAnalysisPass : PassWrapper<FPANAnalysisPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(FPANAnalysisPass)

  FPANAnalysisPass() = default;

  FPANAnalysisPass(const FPANAnalysisPass &other) { instrumentAsserts = other.instrumentAsserts; }

  StringRef getArgument() const final { return "fpan-analysis"; }

  StringRef getDescription() const final { return "FPAN dataflow-analysis"; }

  Option<bool> instrumentAsserts{*this, "instrument-asserts",
                                 llvm::cl::desc("Insert runtime checks for FPAN facts"),
                                 llvm::cl::init(false)};

  void runOnOperation() override {
    DataFlowConfig config;
    config.setInterprocedural(false);

    DataFlowSolver solver(config);
    // DeadCodeAnalysis supplies reachability, without which the solver must
    // assume every branch is taken; SparseConstantPropagation resolves branch
    // conditions for it.  Both are prerequisites, not extras.
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();
    solver.load<mlir_analysis_pass::fpan::FPANAnalysis>();

    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation().emitError("FPAN analysis failed to reach a fixed point");
      return signalPassFailure();
    }

    // Query states only now that the solver has converged.
    auto describe = [&](Value value, AsmState &asmState) -> std::string {
      const auto *lattice = solver.lookupState<mlir_analysis_pass::fpan::SELattice>(value);
      if (!lattice)
        return {};

      const auto &state = lattice->getValue();
      // Top and bottom say nothing; printing them would bury the real facts.
      if (state.isTop() || state.isBottom())
        return {};

      std::string description;
      llvm::raw_string_ostream os(description);
      value.printAsOperand(os, asmState);
      os << " is " << state;
      return description;
    };

    // stderr, so that mlir-opt's stdout stays the unmodified IR and the two can
    // be redirected independently.
    mlir_analysis_pass::annotate::printAnnotated(getOperation(), describe, llvm::errs());

    if (instrumentAsserts)
      insertFPANAssertions(getOperation(), solver);
    else
      markAllAnalysesPreserved();
  }

private:
  static constexpr StringLiteral kAssertFunctionName = "__fpan__assert_f32_state";

  struct PendingCheck {
    mlir::Value value;
    mlir_analysis_pass::fpan::SEState state;
  };

  void insertFPANAssertions(mlir::ModuleOp module, mlir::DataFlowSolver &solver) {
    SmallVector<PendingCheck> checks;

    module.walk([&](Operation *op) {
      if (!op->getParentOfType<LLVM::LLVMFuncOp>())
        return;

      for (Value value : op->getResults()) {
        if (!value.getType().isF32())
          continue;

        auto *lattice = solver.lookupState<mlir_analysis_pass::fpan::SELattice>(value);
        if (!lattice)
          continue;

        const auto &state = lattice->getValue();
        if (!state.isTop() && !state.isBottom())
          checks.push_back({value, state});
      }
    });

    if (checks.empty())
      return;

    LLVM::LLVMFuncOp assertFunction = module.lookupSymbol<LLVM::LLVMFuncOp>(kAssertFunctionName);

    if (!assertFunction) {
      MLIRContext *context = module.getContext();

      SmallVector<Type, 9> argumentTypes;
      argumentTypes.push_back(Float32Type::get(context));
      argumentTypes.append(mlir_analysis_pass::fpan::kEncodedArraySize,
                           IntegerType::get(context, 64));

      auto functionType = LLVM::LLVMFunctionType::get(context, LLVM::LLVMVoidType::get(context),
                                                      argumentTypes, false);

      OpBuilder moduleBuilder(module.getBodyRegion());
      moduleBuilder.setInsertionPointToStart(module.getBody());
      assertFunction = LLVM::LLVMFuncOp::create(moduleBuilder, UnknownLoc::get(context),
                                                kAssertFunctionName, functionType);
    }

    for (const PendingCheck &check : checks) {
      Operation *definition = check.value.getDefiningOp();
      if (!definition)
        continue;

      OpBuilder builder(definition);
      builder.setInsertionPointAfter(definition);

      SmallVector<Value, 9> operands;
      operands.push_back(check.value);
      for (std::uint64_t word : check.state.encodeToArray()) {
        operands.push_back(LLVM::ConstantOp::create(builder, definition->getLoc(),
                                                    IntegerType::get(module.getContext(), 64),
                                                    llvm::APInt(64, word))
                               .getResult());
      }

      LLVM::CallOp::create(builder, definition->getLoc(), assertFunction, operands);
    }
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo mlirGetPassPluginInfo() {
  // LLVM_VERSION_STRING is baked in at compile time and checked by mlir-opt at
  // load time, which is what turns an ABI mismatch into a clear diagnostic.
  return {MLIR_PLUGIN_API_VERSION, "FPANAnalysis", LLVM_VERSION_STRING,
          []() { PassRegistration<FPANAnalysisPass>(); }};
}
