//===- FPANDomain.h - The abstract domain ---------------------------------===//
// TODO
//===----------------------------------------------------------------------===//

#ifndef FPAN_DOMAIN_H
#define FPAN_DOMAIN_H

#include "llvm/Support/raw_ostream.h"

namespace mlir_analysis_pass::fpan {

enum class Kind { Bottom, Zero, NonZero, Top };

inline const char *name(Kind kind) {
  switch (kind) {
  case Kind::Bottom:
    return "bottom";
  case Kind::Zero:
    return "zero";
  case Kind::NonZero:
    return "nonzero";
  case Kind::Top:
    return "top";
  }
  return "top";
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
