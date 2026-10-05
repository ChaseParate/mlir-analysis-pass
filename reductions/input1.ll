declare { float, float } @__fpan_twosum_f32(float, float)

define float @test1() {
  %pair = call { float, float } @__fpan_twosum_f32(float -0.0, float 0.0)
  %sum = extractvalue { float, float } %pair, 0
  %error = extractvalue { float, float } %pair, 1
  %result = fadd float %sum, %error
  ret float %result
}
