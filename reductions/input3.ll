; A longer dataflow chain that combines Z2 and SE-I. The final call uses the
; nonzero SE-I error with -0.0, returning that nonzero value as its sum.
declare { float, float } @__fpan_twosum_f32(float, float)

define float @mixed_lemma_chain(i1 %use_initial_sum) {
  %initial = call { float, float } @__fpan_twosum_f32(float 1.0, float 0.0)
  %initial_sum = extractvalue { float, float } %initial, 0
  %initial_error = extractvalue { float, float } %initial, 1

  %with_negative_zero = call { float, float } @__fpan_twosum_f32(float %initial_sum, float -0.0)
  %zero_sum = extractvalue { float, float } %with_negative_zero, 0
  %zero_error = extractvalue { float, float } %with_negative_zero, 1

  ; Both incoming values have the same FPAN fact, so their PHI preserves the
  ; input needed by SE-I while exercising LLVM control flow.
  br i1 %use_initial_sum, label %initial_path, label %zero_path

initial_path:
  br label %selected_path

zero_path:
  br label %selected_path

selected_path:
  %selected_sum = phi float [ %initial_sum, %initial_path ], [ %zero_sum, %zero_path ]
  %sei = call { float, float } @__fpan_twosum_f32(float %selected_sum, float 5.96046448e-08)
  %sei_sum = extractvalue { float, float } %sei, 0
  %sei_error = extractvalue { float, float } %sei, 1

  %recover_error = call { float, float } @__fpan_twosum_f32(float %sei_error, float -0.0)
  %recovered_sum = extractvalue { float, float } %recover_error, 0
  %recovered_error = extractvalue { float, float } %recover_error, 1

  %same_scale = fcmp oge float %sei_sum, %recovered_sum
  %candidate = fadd float %sei_error, %recovered_error
  %result = select i1 %same_scale, float %candidate, float %recovered_sum
  ret float %result
}
