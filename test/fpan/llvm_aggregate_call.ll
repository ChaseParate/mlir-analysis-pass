declare { float, float } @__fpan_twosum_f32(float, float)

define { float, float, float, float } @twosum_nonzero_zeroes() {
  %sum_pos_pair = call { float, float } @__fpan_twosum_f32(float 1.0, float 0.0)
  %sum_pos = extractvalue { float, float } %sum_pos_pair, 0
  %error_pos = extractvalue { float, float } %sum_pos_pair, 1
  %sum_neg_pair = call { float, float } @__fpan_twosum_f32(float 1.0, float -0.0)
  %sum_neg = extractvalue { float, float } %sum_neg_pair, 0
  %error_neg = extractvalue { float, float } %sum_neg_pair, 1
  %r0 = insertvalue { float, float, float, float } poison, float %sum_pos, 0
  %r1 = insertvalue { float, float, float, float } %r0, float %error_pos, 1
  %r2 = insertvalue { float, float, float, float } %r1, float %sum_neg, 2
  %r3 = insertvalue { float, float, float, float } %r2, float %error_neg, 3
  ret { float, float, float, float } %r3
}

define { float, float, float, float, float, float } @twosum_zero_family() {
  %pp_pair = call { float, float } @__fpan_twosum_f32(float 0.0, float 0.0)
  %pp_sum = extractvalue { float, float } %pp_pair, 0
  %pp_error = extractvalue { float, float } %pp_pair, 1
  %pn_pair = call { float, float } @__fpan_twosum_f32(float 0.0, float -0.0)
  %pn_sum = extractvalue { float, float } %pn_pair, 0
  %pn_error = extractvalue { float, float } %pn_pair, 1
  %nn_pair = call { float, float } @__fpan_twosum_f32(float -0.0, float -0.0)
  %nn_sum = extractvalue { float, float } %nn_pair, 0
  %nn_error = extractvalue { float, float } %nn_pair, 1
  %r0 = insertvalue { float, float, float, float, float, float } poison, float %pp_sum, 0
  %r1 = insertvalue { float, float, float, float, float, float } %r0, float %pp_error, 1
  %r2 = insertvalue { float, float, float, float, float, float } %r1, float %pn_sum, 2
  %r3 = insertvalue { float, float, float, float, float, float } %r2, float %pn_error, 3
  %r4 = insertvalue { float, float, float, float, float, float } %r3, float %nn_sum, 4
  %r5 = insertvalue { float, float, float, float, float, float } %r4, float %nn_error, 5
  ret { float, float, float, float, float, float } %r5
}
