#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// `s0`-`s7` are provided by `SEState::encodeToArray`
extern "C" void __fpan__assert_f32_state(float value, std::uint64_t s0, std::uint64_t s1,
                                           std::uint64_t s2, std::uint64_t s3,
                                           std::uint64_t s4, std::uint64_t s5,
                                           std::uint64_t s6, std::uint64_t s7) {
  std::uint64_t stateWords[] = {s0, s1, s2, s3, s4, s5, s6, s7};

  std::uint32_t bits;
  std::memcpy(&bits, &value, sizeof(float));

  bool negative = (bits >> 31) != 0;
  std::uint32_t exponentField = (bits >> 23) & 0xff;
  
  std::size_t stateBit;
  if ((bits & 0x7fffffffUL) == 0) {
    stateBit = negative ? 1 : 0;
  } else {
    if (exponentField == 0 || exponentField == 255)
      return; // Not handling subnormals, NaNs, or infinities.

    stateBit = exponentField * 2 + negative;
  }

  if ((stateWords[stateBit / 64] & (1UL << (stateBit % 64))) == 0) {
    std::fprintf(stderr, "FPAN assertion failed: f32 bits = 0x%08x\n", bits);
    std::abort();
  }
}
