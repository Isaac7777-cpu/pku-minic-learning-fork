#include "reg.hpp"
#include <stdexcept>
#include <variant>

#include "util/variant_template.hpp"

namespace riscv {

bool operator==(const Reg &lhs, const Reg &rhs) { return lhs.v_ == rhs.v_; }
bool operator!=(const Reg &lhs, const Reg &rhs) { return !(lhs == rhs); }
std::ostream &operator<<(std::ostream &os, const Reg &r) {
  os << r.to_string();
  return os;
}

std::string Reg::to_string() const {
  return std::visit(Overloaded{
                        [](const Zero &) { return std::string("x0"); },
                        [](const Ra &) { return std::string("ra"); },
                        [](const Sp &) { return std::string("sp"); },
                        [](const Gp &) { return std::string("gp"); },
                        [](const Tp &) { return std::string("tp"); },
                        [](const T &r) { return "t" + std::to_string(r.idx); },
                        [](const A &r) { return "a" + std::to_string(r.idx); },
                        [](const S &r) { return "s" + std::to_string(r.idx); },
                    },
                    v_);
}

unsigned int Reg::idx() const {
  return std::visit(
      [](auto const &r) -> unsigned int {
        using R = std::decay_t<decltype(r)>;
        if constexpr (std::is_same_v<R, T> || std::is_same_v<R, A> ||
                      std::is_same_v<R, S>) {
          return r.idx;
        } else {
          throw std::runtime_error("Reg::idx() only valid for T/A/S");
        }
      },
      v_);
}

char Reg::series() const {
  return std::visit(
      [](auto const &r) -> char {
        using R = std::decay_t<decltype(r)>;
        if constexpr (std::is_same_v<R, T>) {
          return 't';
        } else if (std::is_same_v<R, A>) {
          return 'a';
        } else if (std::is_same_v<R, S>) {
          return 's';
        } else {
          throw std::runtime_error("Reg::series() only valid for T/A/S");
        }
      },
      v_);
}

} // namespace riscv
