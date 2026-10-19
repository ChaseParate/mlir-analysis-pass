struct TwoSumResult {
  float sum;
  float error;
};

extern "C" TwoSumResult __fpan_twosum_f32(float x, float y) {
  float sum = x + y;
  float z = sum - x;
  float error = (x - (sum - z)) + (y - z);
  return {sum, error};
}
