#include "codegen_ctx.hpp"
#include "logger.hpp"

bool CodeGenCtx::is_avail(const reg_t &reg) {
  if (reg.series == 't') {
    return this->t_reg[reg.idx];
  } else if (reg.series == 'a') {
    return this->a_reg[reg.idx];
  } else {
    LOG_ERROR("Unrecognised register series.");
  }
}

// This will return a free register in the following order:
// 1. T-registers
// 2. A-registers
// but does not guarantee which one is free first.
std::optional<reg_t> CodeGenCtx::get_avail() {
  std::optional<reg_t> t_reg = get_avail('t');
  if (!t_reg) {
    return get_avail('a');
  } else {
    return t_reg;
  }
}

std::optional<reg_t> CodeGenCtx::get_avail(const char &series) {
  if (series == 't') {
    for (int i = 0; i < 8; i++) {
      if (!t_reg[i]) {
        t_reg[i] = true;
        return reg_t{'t', i};
      }
    }
    return std::nullopt;
  } else if (series == 'a') {
    for (int i = 1; i < 9; i++) {
      if (!a_reg[i % 8]) {
        a_reg[i % 8] = true;
        return reg_t{'a', i};
      }
    }
    return std::nullopt;
  } else {
    LOG_ERROR("Unexpected register series.");
  }
}
