#include <cassert>
#include <koopa.h>
#include <string_view>

#include "backend/asm.hpp"
#include "backend/codegen.hpp"
#include "backend/reg.hpp"
#include "util/logger.hpp"

static constexpr const std::string_view INDENT = "\t";
static const riscv::Reg RETURN_REGISTER = riscv::Reg(riscv::A{0});
static const riscv::Reg ZERO_REGISTER = riscv::Reg(riscv::Zero{});

[[noreturn]] riscv::Reg reallocate_register() {
  LOG_ERROR("Insufficient registers (currently have no way to resolve.)");
}

void CodeGenUnit::generate(const koopa_raw_program_t &program) {
  Visit(program);
}

void CodeGenUnit::emit_inst(const riscv::AsmInst &inst) {
  this->output << INDENT << inst.to_string() << std::endl;
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

riscv::Reg CodeGenUnit::Visit(const koopa_raw_value_t &value) {
  // Check if we have seen the instruction before, avoid duplication
  auto it = this->ctx->reg_dict.find(value);
  if (it != this->ctx->reg_dict.end()) {
    return it->second;
  }

  const auto &kind = value->kind;
  riscv::Reg dst = [&] {
    switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      return Visit(kind.data.ret);
    case KOOPA_RVT_INTEGER:
      return Visit(kind.data.integer);
    case KOOPA_RVT_BINARY:
      return Visit(kind.data.binary);
    default:
      LOG_ERROR("Unexpected koopa instruction type.");
    }
  }();

  // Remember the variable name
  this->ctx->reg_dict.insert_or_assign(value, dst);
  return dst;
}

riscv::Reg CodeGenUnit::Visit(const koopa_raw_return_t &ret) {
  riscv::Reg dst = Visit(ret.value);
  if (dst != RETURN_REGISTER) {
    riscv::AsmInst inst = riscv::AsmInst(riscv::Mv{RETURN_REGISTER, dst});
    this->emit_inst(inst);
  }
  this->emit_inst(riscv::AsmInst(riscv::Ret{}));
  return RETURN_REGISTER;
}

riscv::Reg CodeGenUnit::Visit(const koopa_raw_integer_t &num) {
  // Escape early if it is zero, just use the zero register
  if (num.value == 0) {
    return ZERO_REGISTER;
  }

  std::optional<riscv::Reg> dst = this->ctx->get_avail();
  if (dst) {
    riscv::AsmInst inst = riscv::AsmInst(riscv::Li{dst.value(), num.value});
    this->emit_inst(inst);
    return dst.value();
  } else {
    reallocate_register();
  }
}

riscv::Reg CodeGenUnit::Visit(const koopa_raw_binary_t &binary) {
  riscv::Reg l_reg = Visit(binary.lhs);
  riscv::Reg r_reg = Visit(binary.rhs);
  // HACK: I feel like it is always possible to reuse one of the registers now
  //       since we only have one expression to parse.
  riscv::Reg dst = [&] {
    if (l_reg != ZERO_REGISTER) {
      return l_reg;
    } else if (r_reg != ZERO_REGISTER) {
      return r_reg;
    } else {
      std::optional<riscv::Reg> dst_req = this->ctx->get_avail();
      if (dst_req) {
        return dst_req.value();
      } else {
        reallocate_register();
      }
    }
  }();
  switch (binary.op) {
  case KOOPA_RBO_EQ: {
    // Build XOR -> SEQZ instruction
    riscv::AsmInst xor_inst = riscv::AsmInst(riscv::Xor{dst, l_reg, r_reg});
    riscv::AsmInst seqz_inst = riscv::AsmInst(riscv::Seqz{dst, dst});

    // Emit instruction
    this->emit_inst(xor_inst);
    this->emit_inst(seqz_inst);
    return dst;
  }
  case KOOPA_RBO_NOT_EQ: {
    // xor instructions -> snez (to normalise to 0/1)
    riscv::AsmInst xor_inst = riscv::AsmInst(riscv::Xor{dst, l_reg, r_reg});
    riscv::AsmInst snez_inst = riscv::AsmInst(riscv::Snez{dst, dst});

    // Emit instruction
    this->emit_inst(xor_inst);
    this->emit_inst(snez_inst);
    return dst;
  }
  case KOOPA_RBO_LT: {
    // slt instruction
    riscv::AsmInst slt_inst = riscv::AsmInst(riscv::Slt{dst, l_reg, r_reg});

    // Emit instruction
    this->emit_inst(slt_inst);
    return dst;
  }
  case KOOPA_RBO_LE: {
    // sgt instruction (inverse slt) -> xor with 1 to take negation
    // (a <= b <=> !(a > b)
    riscv::AsmInst sgt_inst = riscv::AsmInst(riscv::Sgt{dst, l_reg, r_reg});
    riscv::AsmInst xor_inst = riscv::AsmInst(riscv::Xori{dst, dst, 1});

    // emit instruction
    this->emit_inst(sgt_inst);
    this->emit_inst(xor_inst);
    return dst;
  }
  case KOOPA_RBO_GT: {
    // sgt instruction (inverse slt) (a > b <=> b < a)
    riscv::AsmInst sgt_inst = riscv::AsmInst(riscv::Sgt{dst, l_reg, r_reg});

    // Emit instruction
    this->emit_inst(sgt_inst);
    return dst;
  }
  case KOOPA_RBO_GE: {
    // slt instruction -> xor with 1 to take negation (a >= b <=> !(a < b))
    riscv::AsmInst slt_inst = riscv::AsmInst(riscv::Slt{dst, l_reg, r_reg});
    riscv::AsmInst xor_inst = riscv::AsmInst(riscv::Xori{dst, dst, 1});

    // emit instruction
    this->emit_inst(slt_inst);
    this->emit_inst(xor_inst);
    return dst;
  }
  case KOOPA_RBO_AND: {
    // Since Koopa `and` inst is already bitwise, use `and` in riscv
    riscv::AsmInst and_inst = riscv::AsmInst(riscv::And{dst, l_reg, r_reg});

    // emit instruction
    this->emit_inst(and_inst);
    return dst;
  }
  case KOOPA_RBO_OR: {
    // Since Koopa `or` inst is already bitwise, use `or` in riscv
    riscv::AsmInst or_inst = riscv::AsmInst(riscv::Or{dst, l_reg, r_reg});

    // emit instruction
    this->emit_inst(or_inst);
    return dst;
  }
  case KOOPA_RBO_ADD: {
    riscv::AsmInst add_inst = riscv::AsmInst(riscv::Add{dst, l_reg, r_reg});
    this->emit_inst(add_inst);
    return dst;
  }
  case KOOPA_RBO_SUB: {
    riscv::AsmInst sub_inst = riscv::AsmInst(riscv::Sub{dst, l_reg, r_reg});
    this->emit_inst(sub_inst);
    return dst;
  }
  case KOOPA_RBO_MUL: {
    riscv::AsmInst mul_inst = riscv::AsmInst(riscv::Mul{dst, l_reg, r_reg});
    this->emit_inst(mul_inst);
    return dst;
  }
  case KOOPA_RBO_DIV: {
    riscv::AsmInst mul_inst = riscv::AsmInst(riscv::Div{dst, l_reg, r_reg});
    this->emit_inst(mul_inst);
    return dst;
  }
  case KOOPA_RBO_MOD: {
    riscv::AsmInst mul_inst = riscv::AsmInst(riscv::Mod{dst, l_reg, r_reg});
    this->emit_inst(mul_inst);
    return dst;
  }
  case KOOPA_RBO_XOR: {
    riscv::AsmInst xor_inst = riscv::AsmInst(riscv::Xor{dst, l_reg, r_reg});
    this->emit_inst(xor_inst);
    return dst;
  }
  default:
    LOG_ERROR("Unimplemented koopa binary operations...");
  }
}
