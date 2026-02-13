#pragma once

#include <optional>
#include <unordered_map>
#include "koopa.h"
#include "reg.hpp"

class CodeGenCtx {
private:
  // If set to true, it is being used.
  bool t_reg[7];
  bool a_reg[8];

public:
  std::unordered_map<koopa_raw_value_t, riscv::Reg> reg_dict;

  bool is_avail(const riscv::Reg &);
  std::optional<riscv::Reg> get_avail();
  std::optional<riscv::Reg> get_avail(const char &);
};
