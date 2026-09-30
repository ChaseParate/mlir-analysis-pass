module {
  llvm.func @pos_zero() -> f32 {
    %0 = llvm.mlir.constant(0.000000e+00 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @neg_zero() -> f32 {
    %0 = llvm.mlir.constant(-0.000000e+00 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @pos_one() -> f32 {
    %0 = llvm.mlir.constant(1.000000e+00 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @neg_one() -> f32 {
    %0 = llvm.mlir.constant(-1.000000e+00 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @pos_half() -> f32 {
    %0 = llvm.mlir.constant(5.000000e-01 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @neg_six_point_five() -> f32 {
    %0 = llvm.mlir.constant(-6.500000e+00 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @pos_min_normal() -> f32 {
    %0 = llvm.mlir.constant(1.17549435e-38 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @neg_max_normal() -> f32 {
    %0 = llvm.mlir.constant(-3.40282347e+38 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @unhandled_add() -> f32 {
    %0 = llvm.mlir.constant(1.000000e+00 : f32) : f32
    %1 = llvm.mlir.constant(2.000000e+00 : f32) : f32
    %2 = llvm.fadd %0, %1 : f32
    llvm.return %2 : f32
  }

  llvm.func @nan_is_untracked() -> f32 {
    %0 = llvm.mlir.constant(0x7FC00000 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @pos_inf_is_untracked() -> f32 {
    %0 = llvm.mlir.constant(0x7F800000 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @neg_inf_is_untracked() -> f32 {
    %0 = llvm.mlir.constant(0xFF800000 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @denormal_is_untracked() -> f32 {
    %0 = llvm.mlir.constant(0x00000001 : f32) : f32
    llvm.return %0 : f32
  }

  llvm.func @f64_is_untracked() -> f64 {
    %0 = llvm.mlir.constant(1.000000e+300 : f64) : f64
    llvm.return %0 : f64
  }
}
