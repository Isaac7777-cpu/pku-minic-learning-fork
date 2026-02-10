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
  std::unordered_map<koopa_raw_value_t, rv::Reg> reg_dict;

  bool is_avail(const rv::Reg &);
  std::optional<rv::Reg> get_avail();
  std::optional<rv::Reg> get_avail(const char &);
};
