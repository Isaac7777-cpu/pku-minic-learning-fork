#include "codegen.hpp"
#include "koopa.h"
#include "logger.hpp"

#include <cassert>
#include <string_view>

static constexpr const std::string_view INDENT = "\t";
static constexpr const reg_t RETURN_REGISTER = reg_t{'a', 0};
static constexpr const reg_t ZERO_REGISTER = reg_t{'x', 0};

reg_t reallocate_register() {
  LOG_ERROR("Insufficient registers (currently have no way to resolve.)");
}

void CodeGenUnit::generate(const koopa_raw_program_t &program) {
  Visit(program);
}

void CodeGenUnit::Visit(const koopa_raw_program_t &program) {
  Visit(program.values);
  // Mark the start of programs
  output << INDENT << ".text" << std::endl
         << INDENT << ".global main" << std::endl;
  Visit(program.funcs);
}

void CodeGenUnit::Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; i++) {
    auto ptr = slice.buffer[i];

    switch (slice.kind) {
    case KOOPA_RSIK_FUNCTION:
      Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
      break;
    case KOOPA_RSIK_BASIC_BLOCK:
      Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
      break;
    case KOOPA_RSIK_VALUE:
      Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
      break;
    default:
      assert(false);
    }
  }
}

void CodeGenUnit::Visit(const koopa_raw_function_t &func) {
  // Put the name (may need to add arguments later)
  output << std::string_view(func->name).substr(1) << ":" << std::endl;

  // Visit the function body
  Visit(func->bbs);
}

void CodeGenUnit::Visit(const koopa_raw_basic_block_t &basic_block) {
  indent_level++;

  Visit(basic_block->insts);

  indent_level--;
}

reg_t CodeGenUnit::Visit(const koopa_raw_value_t &value) {
  // Check if we have seen the instruction before, avoid duplication
  auto it = this->ctx->reg_dict.find(value);
  if (it != this->ctx->reg_dict.end()) {
    return it->second;
  }

  const auto &kind = value->kind;
  reg_t dst;
  switch (kind.tag) {
  case KOOPA_RVT_RETURN:
    dst = Visit(kind.data.ret);
    break;
  case KOOPA_RVT_INTEGER:
    dst = Visit(kind.data.integer);
    break;
  case KOOPA_RVT_BINARY:
    dst = Visit(kind.data.binary);
    break;
  default:
    LOG_ERROR("Unexpected koopa instruction type.");
  }

  // Remember the variable name
  this->ctx->reg_dict[value] = dst;
  return dst;
}

reg_t CodeGenUnit::Visit(const koopa_raw_return_t &ret) {
  reg_t dst = Visit(ret.value);
  if (dst != RETURN_REGISTER) {
    output << INDENT << "mv    a0, " << dst.to_string() << std::endl;
  }
  output << INDENT << "ret" << std::endl;
  return reg_t{'a', 0};
}

reg_t CodeGenUnit::Visit(const koopa_raw_integer_t &num) {
  // Escape early if it is zero, just use the zero register
  if (num.value == 0) {
    return ZERO_REGISTER;
  }

  std::optional<reg_t> dst = this->ctx->get_avail();
  if (dst) {
    output << INDENT << "li    " << dst->to_string() << ", " << num.value
           << std::endl;
    return *dst;
  } else {
    reallocate_register();
  }
}

reg_t CodeGenUnit::Visit(const koopa_raw_binary_t &binary) {
  reg_t l_reg = Visit(binary.lhs);
  reg_t r_reg = Visit(binary.rhs);
  reg_t dst;
  switch (binary.op) {
  case KOOPA_RBO_EQ: {
    // Reuse one of the register
    if (l_reg != ZERO_REGISTER) {
      dst = l_reg;
    } else if (r_reg != ZERO_REGISTER) {
      dst = r_reg;
    } else {
      std::optional<reg_t> dst_req = this->ctx->get_avail();
      if (dst_req) {
        dst = dst_req.value();
      } else {
        reallocate_register();
      }
    }

    // XOR instruction
    this->output << INDENT << "xor   " << dst.to_string() << ", "
                 << l_reg.to_string() << ", " << r_reg.to_string() << std::endl;
    // SEQZ instruction
    this->output << INDENT << "seqz  " << dst.to_string() << ", "
                 << dst.to_string() << std::endl;
    return dst;
    break;
  }
  case KOOPA_RBO_SUB: {
    std::optional<reg_t> dst_req = this->ctx->get_avail();
    if (dst_req) {
      dst = dst_req.value();
    } else {
      reallocate_register();
    }
    this->output << INDENT << "sub   " << dst.to_string() << ", "
                 << l_reg.to_string() << ", " << r_reg.to_string() << std::endl;
    return dst;
    break;
  }
  case KOOPA_RBO_XOR: {
    std::optional<reg_t> dst_req = this->ctx->get_avail();
    if (dst_req) {
      dst = dst_req.value();
    } else {
      reallocate_register();
    }
    this->output << INDENT << "xor   " << dst.to_string() << ", "
                 << l_reg.to_string() << ", " << r_reg.to_string() << std::endl;
    return dst;
    break;
  }
  default:
    LOG_ERROR("Unimplemented...");
  }
}
