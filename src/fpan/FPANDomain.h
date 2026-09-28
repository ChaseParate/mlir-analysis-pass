//===- FPANDomain.h - The abstract domain ---------------------------------===//
// TODO
//===----------------------------------------------------------------------===//

#ifndef FPAN_DOMAIN_H
#define FPAN_DOMAIN_H

#include <cstdint>
#include <string>
#include <utility>

#include "llvm/Support/raw_ostream.h"

namespace mlir_analysis_pass::fpan {

enum class Kind : std::uint8_t {
  Bottom = 0,

  Negative = 1 << 0,
  Zero = 1 << 1,
  Positive = 1 << 2,

  Top = Negative | Zero | Positive,
};

constexpr Kind operator|(const Kind &a, const Kind &b) {
  return static_cast<Kind>(static_cast<std::uint8_t>(a) |
                           static_cast<std::uint8_t>(b));
}

constexpr Kind &operator|=(Kind &self, const Kind &other) {
  return self = self | other;
}

static inline std::string name(const Kind &kind) {
  if (kind == Kind::Bottom)
    return "bottom";
  if (kind == Kind::Top)
    return "top";

  constexpr std::pair<Kind, const char *> flagLabels[] = {
      {Kind::Negative, "negative"},
      {Kind::Zero, "zero"},
      {Kind::Positive, "positive"},
  };

  std::string result;
  const std::uint8_t bits = static_cast<std::uint8_t>(kind);
  for (const auto &[flag, label] : flagLabels) {
    if (bits & static_cast<std::uint8_t>(flag)) {
      if (!result.empty())
        result += " or ";
      result += label;
    }
  }

  return result;
}

struct FPANState {
  Kind kind = Kind::Bottom;

  FPANState() = default;
  /* implicit */ FPANState(Kind kind) : kind(kind) {}

  static FPANState bottom() { return Kind::Bottom; }
  static FPANState top() { return Kind::Top; }

  bool isBottom() const { return kind == Kind::Bottom; }

  /// Least upper bound.  Two disagreeing facts lose all information.
  static FPANState join(const FPANState &lhs, const FPANState &rhs) {
    if (lhs.kind == Kind::Bottom)
      return rhs;
    if (rhs.kind == Kind::Bottom)
      return lhs;
    if (lhs.kind == rhs.kind)
      return lhs;
    return top();
  }

  bool operator==(const FPANState &other) const { return kind == other.kind; }
  bool operator!=(const FPANState &other) const { return kind != other.kind; }

  void print(llvm::raw_ostream &os) const { os << name(kind); }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const FPANState &state) {
  state.print(os);
  return os;
}

} // namespace mlir_analysis_pass::fpan

#endif
