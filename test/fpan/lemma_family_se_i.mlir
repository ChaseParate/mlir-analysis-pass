module {
  func.func private @__fpan_twosum_f32(f32, f32) -> (f32, f32)

  func.func @twosum_se_i() -> (f32, f32, f32, f32, f32, f32, f32, f32) {
    %one = llvm.mlir.constant(1.000000e+00 : f32) : f32
    %within_range = llvm.mlir.constant(5.96046448e-08 : f32) : f32
    %boundary_same_sign = llvm.mlir.constant(2.98023224e-08 : f32) : f32
    %boundary_different_sign = llvm.mlir.constant(-2.98023224e-08 : f32) : f32
    %outside_range = llvm.mlir.constant(1.49011612e-08 : f32) : f32

    %within_sum, %within_error = func.call @__fpan_twosum_f32(%one, %within_range) : (f32, f32) -> (f32, f32)
    %boundary_sum, %boundary_error = func.call @__fpan_twosum_f32(%one, %boundary_same_sign) : (f32, f32) -> (f32, f32)
    %different_sum, %different_error = func.call @__fpan_twosum_f32(%one, %boundary_different_sign) : (f32, f32) -> (f32, f32)
    %outside_sum, %outside_error = func.call @__fpan_twosum_f32(%one, %outside_range) : (f32, f32) -> (f32, f32)
    func.return %within_sum, %within_error, %boundary_sum, %boundary_error, %different_sum, %different_error, %outside_sum, %outside_error : f32, f32, f32, f32, f32, f32, f32, f32
  }
}
