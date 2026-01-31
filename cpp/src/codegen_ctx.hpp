#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include "koopa.h"

struct reg_t {
  char series;
  int idx;

  bool operator==(const reg_t &other) const {
    return (series == other.series && idx == other.idx);
  }

  bool operator!=(const reg_t &other) const { return !(*this == other); }

  std::string to_string() const {
    return std::string(1, series) + std::to_string(idx);
  }
};

class CodeGenCtx {
private:
  // If set to true, it is being used.
  bool t_reg[7];
  bool a_reg[8];

public:
  std::unordered_map<koopa_raw_value_t, reg_t> reg_dict;

  bool is_avail(const reg_t &);
  std::optional<reg_t> get_avail();
  std::optional<reg_t> get_avail(const char &);
};
