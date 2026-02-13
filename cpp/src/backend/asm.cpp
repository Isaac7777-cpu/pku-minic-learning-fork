#include "asm.hpp"
#include <sstream>
#include <string>
#include <variant>

#include "util/variant_template.hpp"

namespace riscv {

std::ostream &operator<<(std::ostream &os, const AsmInst &inst) {
  return os << inst.to_string();
}

std::string AsmInst::to_string() const {
  return std::visit(
      Overloaded{
          [](const Li &inst) {
            std::stringstream ss;
            ss << "li    " << inst.rd << ", " << inst.imm;
            return ss.str();
          },
          [](const Mv &inst) {
            std::stringstream ss;
            ss << "mv    " << inst.rd << ", " << inst.rs;
            return ss.str();
          },
          [](const Add &inst) {
            std::stringstream ss;
            ss << "add   " << inst.rd << ", " << inst.rs1 << ", " << inst.rs2;
            return ss.str();
          },
          [](const Sub &inst) {
            std::stringstream ss;
            ss << "sub   " << inst.rd << ", " << inst.rs1 << ", " << inst.rs2;
            return ss.str();
          },
          [](const Mul &inst) {
            std::stringstream ss;
            ss << "mul   " << inst.rd << ", " << inst.rs1 << ", " << inst.rs2;
            return ss.str();
          },
          [](const Div &inst) {
            std::stringstream ss;
            ss << "div   " << inst.rd << ", " << inst.rs1 << ", " << inst.rs2;
            return ss.str();
          },
          [](const Mod &inst) {
            std::stringstream ss;
            ss << "mod   " << inst.rd << ", " << inst.rs1 << ", " << inst.rs2;
            return ss.str();
          },
          [](const Xor &inst) {
            std::stringstream ss;
            ss << "xor   " << inst.rd << ", " << inst.rs1 << ", " << inst.rs2;
            return ss.str();
          },
          [](const Seqz &inst) {
            std::stringstream ss;
            ss << "seqz  " << inst.rd << ", " << inst.rs;
            return ss.str();
          },
          [](const Ret &) { return std::string("ret"); },
      },
      this->inst_);
};
} // namespace riscv
