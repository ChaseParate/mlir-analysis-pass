; A chained signed-zero case. Every TwoSum result is immediately unpacked,
; then feeds a later aggregate-returning TwoSum call.
declare { float, float } @__fpan_twosum_f32(float, float)

define float @cascaded_zero_recovery(i1 %choose_errors) {
  %first = call { float, float } @__fpan_twosum_f32(float -0.0, float 0.0)
  %first_sum = extractvalue { float, float } %first, 0
  %first_error = extractvalue { float, float } %first, 1

  %second = call { float, float } @__fpan_twosum_f32(float %first_error, float %first_sum)
  %second_sum = extractvalue { float, float } %second, 0
  %second_error = extractvalue { float, float } %second, 1

  %third = call { float, float } @__fpan_twosum_f32(float %second_sum, float %first_error)
  %third_sum = extractvalue { float, float } %third, 0
  %third_error = extractvalue { float, float } %third, 1

  ; Keep the extracted values live through two distinct computation paths.
  %same_sum = fcmp oeq float %first_sum, %third_sum
  br i1 %choose_errors, label %errors, label %sums

errors:
  %error_total = fsub float %second_error, %third_error
  br label %merge

sums:
  %sum_total = fadd float %first_sum, %third_sum
  br label %merge

merge:
  %chosen = phi float [ %error_total, %errors ], [ %sum_total, %sums ]
  %result = select i1 %same_sum, float %chosen, float %first_error
  ret float %result
}
