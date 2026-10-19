module {
  func.func private @__fpan_twosum_f32(f32, f32) -> (f32, f32)

  func.func @twosum_se_s1() -> (f32, f32, f32, f32) {
    %one = llvm.mlir.constant(1.000000e+00 : f32) : f32
    // 2^-24: the identity alternative is realized by the tie-to-even case.
    %half_ulp = llvm.mlir.constant(5.96046448e-08 : f32) : f32
    // 3 * 2^-25 = 0.75 ulp(1): the rounded-up alternative is realized.
    %three_quarter_ulp = llvm.mlir.constant(8.94069672e-08 : f32) : f32

    %tie_sum, %tie_error = func.call @__fpan_twosum_f32(%one, %half_ulp) : (f32, f32) -> (f32, f32)
    %round_sum, %round_error = func.call @__fpan_twosum_f32(%one, %three_quarter_ulp) : (f32, f32) -> (f32, f32)
    func.return %tie_sum, %tie_error, %round_sum, %round_error : f32, f32, f32, f32
  }
}
