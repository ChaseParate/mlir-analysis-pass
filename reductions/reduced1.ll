
declare { float, float } @__fpan_twosum_f32(float, float)

define float @test1() {
  %pair = call { float, float } @__fpan_twosum_f32(float 0.000000e+00, float 0.000000e+00)
  %sum = extractvalue { float, float } %pair, 0
  ret float %sum
}
