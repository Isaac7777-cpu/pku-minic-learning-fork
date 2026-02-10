#pragma once

#include <string>
#include <variant>

namespace rv {

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

private:
  Impl v_;
};

} // namespace rv
