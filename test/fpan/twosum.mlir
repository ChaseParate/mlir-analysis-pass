module {
  func.func private @__fpan_twosum_f32(f32, f32) -> (f32, f32)

  func.func @twosum_nonzero_zeroes() -> (f32, f32, f32, f32) {
    %normal = llvm.mlir.constant(1.000000e+00 : f32) : f32
    %pos_zero = llvm.mlir.constant(0.000000e+00 : f32) : f32
    %neg_zero = llvm.mlir.constant(-0.000000e+00 : f32) : f32
    %sum_pos, %error_pos = func.call @__fpan_twosum_f32(%normal, %pos_zero) : (f32, f32) -> (f32, f32)
    %sum_neg, %error_neg = func.call @__fpan_twosum_f32(%normal, %neg_zero) : (f32, f32) -> (f32, f32)
    func.return %sum_pos, %error_pos, %sum_neg, %error_neg : f32, f32, f32, f32
  }

  func.func @twosum_zero_family() -> (f32, f32, f32, f32, f32, f32) {
    %pos_zero = llvm.mlir.constant(0.000000e+00 : f32) : f32
    %neg_zero = llvm.mlir.constant(-0.000000e+00 : f32) : f32
    %sum_pp, %error_pp = func.call @__fpan_twosum_f32(%pos_zero, %pos_zero) : (f32, f32) -> (f32, f32)
    %sum_pn, %error_pn = func.call @__fpan_twosum_f32(%pos_zero, %neg_zero) : (f32, f32) -> (f32, f32)
    %sum_nn, %error_nn = func.call @__fpan_twosum_f32(%neg_zero, %neg_zero) : (f32, f32) -> (f32, f32)
    func.return %sum_pp, %error_pp, %sum_pn, %error_pn, %sum_nn, %error_nn : f32, f32, f32, f32, f32, f32
  }
}
