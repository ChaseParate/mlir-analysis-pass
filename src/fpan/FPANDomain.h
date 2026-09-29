//===- FPANDomain.h - The abstract domain ---------------------------------===//
// TODO
//===----------------------------------------------------------------------===//

#ifndef FPAN_DOMAIN_H
#define FPAN_DOMAIN_H

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "llvm/Support/raw_ostream.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"

namespace mlir_analysis_pass::fpan {

using Exponent = int8_t; // Assuming `f32`'s for now.

constexpr Exponent kEMin = -126;
constexpr Exponent kEMax = 127;
constexpr Exponent kZeroExponent = kEMin - 1; // Sentinel value for zeroes.
constexpr std::size_t kExponentCount = kEMax - kZeroExponent + 1;

class SEState {
  // Lower bit set means positive, upper bit set means negative. Both bits set means top, both unset means bottom.
  std::bitset<kExponentCount * 2> signsByExponent;

  static inline constexpr std::size_t index(Exponent exponent, bool negative) {
    const std::size_t exponentIndex = exponent - kZeroExponent;
    return exponentIndex * 2 + negative;
  }

public:
  SEState() = default;

  static std::optional<SEState> fromFloat(const llvm::APFloat &f) {
    if (f.isNaN() || f.isInfinity() || f.isDenormal())
      return std::nullopt;

    if (f.isZero()) {
      SEState state;
      state.setSign(kZeroExponent, f.isNegative());
      return state;
    }

    const int exponent = llvm::ilogb(f);
    if (exponent < kEMin || exponent > kEMax) {
      llvm::errs() << "Invalid exponent state in SEState::fromFloat\n";
      return std::nullopt;
    }

    SEState state;
    state.setSign(exponent, f.isNegative());
    return state;
  }

  bool hasSign(Exponent exponent, bool negative) const {
    return signsByExponent.test(index(exponent, negative));
  }

  void setSign(Exponent exponent, bool negative) {
    signsByExponent.set(index(exponent, negative));
  }

  static SEState top() {
    SEState state;
    state.signsByExponent.flip();

    return state;
  }

  static SEState bottom() {
    return {};
  }

  bool isTop() const { return signsByExponent.all(); }
  bool isBottom() const { return signsByExponent.none(); }

  /// Least upper bound. Two disagreeing facts lose all information.
  static SEState join(const SEState &lhs, const SEState &rhs) {
    SEState newState;
    newState.signsByExponent = lhs.signsByExponent | rhs.signsByExponent;

    return newState;
  }

  bool operator==(const SEState &other) const { return signsByExponent == other.signsByExponent; }
  bool operator!=(const SEState &other) const { return signsByExponent != other.signsByExponent; }

  void print(llvm::raw_ostream &os) const {
    os << '{';

    bool first = true;
    for (std::size_t exponentIndex = 0; exponentIndex < kExponentCount; ++exponentIndex) {
      Exponent exponent = static_cast<Exponent>(static_cast<int>(kZeroExponent) + static_cast<int>(exponentIndex));
      bool hasPositive = hasSign(exponent, false), hasNegative = hasSign(exponent, true);

      if (!hasPositive && !hasNegative)
        continue;

      if (!first)
        os << ", ";
      first = false;

      const char *signText = hasPositive && hasNegative
        ? "+/-"
        : hasPositive ? "+" : "-";

      os << '(';
      if (exponent == kZeroExponent)
        os << "ZERO";
      else
        os << static_cast<int>(exponent);
      os << ", " << signText << ')';
    }

    os << '}';
  }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const SEState &state) {
  state.print(os);
  return os;
}

} // namespace mlir_analysis_pass::fpan

#endif
