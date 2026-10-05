
declare { float, float } @__fpan_twosum_f32(float, float)

define float @mixed_lemma_chain() {
initial_path:
  %initial = call { float, float } @__fpan_twosum_f32(float 0.000000e+00, float 0.000000e+00)
  %initial_sum = extractvalue { float, float } %initial, 0
  %initial_error = extractvalue { float, float } %initial, 1
  %with_negative_zero = call { float, float } @__fpan_twosum_f32(float 0.000000e+00, float 0.000000e+00)
  %zero_sum = extractvalue { float, float } %with_negative_zero, 0
  %zero_error = extractvalue { float, float } %with_negative_zero, 1
  %sei = call { float, float } @__fpan_twosum_f32(float 0.000000e+00, float f0x33800000)
  %sei_sum = extractvalue { float, float } %sei, 0
  %sei_error = extractvalue { float, float } %sei, 1
  %recover_error = call { float, float } @__fpan_twosum_f32(float 0.000000e+00, float 0.000000e+00)
  %recovered_sum = extractvalue { float, float } %recover_error, 0
  %recovered_error = extractvalue { float, float } %recover_error, 1
  ret float %recovered_error
}
