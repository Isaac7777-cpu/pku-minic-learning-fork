#pragma once

#include <cstdint>
#include <variant>

#include "reg.hpp"

namespace riscv {
struct Li {
  Reg rd;
  std::int32_t imm;
};

struct Mv {
  Reg rd;
  Reg rs;
};

struct Sub {
  Reg rd;
  Reg rs1;
  Reg rs2;
};

struct Add {
  Reg rd;
  Reg rs1;
  Reg rs2;
};

struct Mul {
  Reg rd;
  Reg rs1;
  Reg rs2;
};

struct Div {
  Reg rd;
  Reg rs1;
  Reg rs2;
};

struct Mod {
  Reg rd;
  Reg rs1;
  Reg rs2;
};

struct Xor {
  Reg rd;
  Reg rs1;
  Reg rs2;
};

struct Seqz {
  Reg rd;
  Reg rs;
};

struct Ret {};

class AsmInst {
public:
  using Impl = std::variant<Li, Mv, Sub, Add, Mul, Div, Mod, Xor, Seqz, Ret>;

  AsmInst(Li inst) : inst_(inst) {}
  AsmInst(Mv inst) : inst_(inst) {}
  AsmInst(Sub inst) : inst_(inst) {}
  AsmInst(Add inst) : inst_(inst) {}
  AsmInst(Mul inst) : inst_(inst) {}
  AsmInst(Div inst) : inst_(inst) {}
  AsmInst(Mod inst) : inst_(inst) {}
  AsmInst(Xor inst) : inst_(inst) {}
  AsmInst(Seqz inst) : inst_(inst) {}
  AsmInst(Ret inst) : inst_(inst) {}

  std::string to_string() const;

  friend std::ostream &operator<<(std::ostream &, const AsmInst &);

private:
  Impl inst_;
};

} // namespace riscv
