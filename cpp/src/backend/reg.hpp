#pragma once

#include <string>
#include <variant>

namespace riscv {

struct Zero {};
struct Ra {};
struct Sp {};
struct Gp {};
struct Tp {};
struct T {
  int idx;
};
struct A {
  int idx;
};
struct S {
  int idx;
};

inline constexpr bool operator==(Zero, Zero) noexcept { return true; }
inline constexpr bool operator==(Ra, Ra) noexcept { return true; }
inline constexpr bool operator==(Sp, Sp) noexcept { return true; }
inline constexpr bool operator==(Gp, Gp) noexcept { return true; }
inline constexpr bool operator==(Tp, Tp) noexcept { return true; }

inline constexpr bool operator==(const T &l, const T &r) {
  return l.idx == r.idx;
}
inline constexpr bool operator==(const A &l, const A &r) {
  return l.idx == r.idx;
}
inline constexpr bool operator==(const S &l, const S &r) {
  return l.idx == r.idx;
}

class Reg {
public:
  using Impl = std::variant<Zero, Ra, Sp, Gp, Tp, T, A, S>;

  Reg(Zero v) : v_(v) {}
  Reg(Ra v) : v_(v) {}
  Reg(Sp v) : v_(v) {}
  Reg(Gp v) : v_(v) {}
  Reg(Tp v) : v_(v) {}
  Reg(T v) : v_(v) {}
  Reg(A v) : v_(v) {}
  Reg(S v) : v_(v) {}

  std::string to_string() const;
  unsigned int idx() const;
  char series() const;

  friend bool operator==(const Reg &lhs, const Reg &rhs);
  friend bool operator!=(const Reg &lhs, const Reg &rhs);
  friend std::ostream &operator<<(std::ostream &os, const Reg &);

private:
  Impl v_;
};

} // namespace riscv
