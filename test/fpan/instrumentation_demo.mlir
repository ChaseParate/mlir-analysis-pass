// End-to-end input for the fpan-instrumentation-demo target.  `main` lives in
// MLIR so calls to the aggregate-returning TwoSum function retain their MLIR
// ABI through conversion to LLVM.
module {
  func.func private @__fpan_twosum_f32(f32, f32) -> (f32, f32)

  func.func @twosum_se_s1() -> (f32, f32, f32, f32) {
    %one = llvm.mlir.constant(1.000000e+00 : f32) : f32
    %half_ulp = llvm.mlir.constant(5.96046448e-08 : f32) : f32
    %three_quarter_ulp = llvm.mlir.constant(8.94069672e-08 : f32) : f32

    %tie_sum, %tie_error = func.call @__fpan_twosum_f32(%one, %half_ulp) : (f32, f32) -> (f32, f32)
    %round_sum, %round_error = func.call @__fpan_twosum_f32(%one, %three_quarter_ulp) : (f32, f32) -> (f32, f32)
    func.return %tie_sum, %tie_error, %round_sum, %round_error : f32, f32, f32, f32
  }

  func.func @main() -> i32 {
    %unused:4 = func.call @twosum_se_s1() : () -> (f32, f32, f32, f32)
    %status = llvm.mlir.constant(0 : i32) : i32
    func.return %status : i32
  }
}
