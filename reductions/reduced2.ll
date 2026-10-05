
declare { float, float } @__fpan_twosum_f32(float, float)

define float @cascaded_zero_recovery() {
  %first = call { float, float } @__fpan_twosum_f32(float 0.000000e+00, float 0.000000e+00)
  %first_sum = extractvalue { float, float } %first, 0
  %first_error = extractvalue { float, float } %first, 1
  %second = call { float, float } @__fpan_twosum_f32(float 0.000000e+00, float 0.000000e+00)
  %second_sum = extractvalue { float, float } %second, 0
  %second_error = extractvalue { float, float } %second, 1
  %third = call { float, float } @__fpan_twosum_f32(float 0.000000e+00, float 0.000000e+00)
  %third_sum = extractvalue { float, float } %third, 0
  %third_error = extractvalue { float, float } %third, 1
  ret float %third_error
}
