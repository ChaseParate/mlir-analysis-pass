module {
  func.func private @__fpan_twosum_f32(f32, f32) -> (f32, f32)

  // Both incoming states meet SE-I's condition with %rhs after they are joined.
  func.func @join_safe_for_se_i(%flag: i1) -> (f32, f32) {
    %large = llvm.mlir.constant(1.000000e+00 : f32) : f32
    %small = llvm.mlir.constant(5.96046448e-08 : f32) : f32
    %rhs = llvm.mlir.constant(2.98023224e-08 : f32) : f32
    cf.cond_br %flag, ^large_edge, ^small_edge
  ^large_edge:
    cf.br ^join(%large : f32)
  ^small_edge:
    cf.br ^join(%small : f32)
  ^join(%merged: f32):
    %sum, %error = func.call @__fpan_twosum_f32(%merged, %rhs) : (f32, f32) -> (f32, f32)
    func.return %sum, %error : f32, f32
  }

  // One incoming state violates SE-I, so it must not be applied to the join.
  func.func @join_rejected_by_se_i(%flag: i1) -> (f32, f32) {
    %large = llvm.mlir.constant(1.000000e+00 : f32) : f32
    %tiny = llvm.mlir.constant(1.49011612e-08 : f32) : f32
    cf.cond_br %flag, ^large_edge, ^tiny_edge
  ^large_edge:
    cf.br ^join(%large : f32)
  ^tiny_edge:
    cf.br ^join(%tiny : f32)
  ^join(%merged: f32):
    %sum, %error = func.call @__fpan_twosum_f32(%merged, %large) : (f32, f32) -> (f32, f32)
    func.return %sum, %error : f32, f32
  }
}
