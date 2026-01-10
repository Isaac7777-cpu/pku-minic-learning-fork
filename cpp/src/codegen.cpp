#include "codegen.hpp"
#include "koopa.h"
#include <cassert>
#include <string_view>

static constexpr const std::string_view INDENT = "\t";

void CodeGenUnit::generate(const koopa_raw_program_t &program) { Visit(program); }

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

void CodeGenUnit::Visit(const koopa_raw_value_t &value) {
  const auto &kind = value->kind;
  switch (kind.tag) {
  case KOOPA_RVT_RETURN:
    Visit(kind.data.ret);
    break;
  case KOOPA_RVT_INTEGER:
    Visit(kind.data.integer);
    break;
  default:
    assert(false && "Unexpected koopa instruction type.");
  }
}

void CodeGenUnit::Visit(const koopa_raw_return_t &ret) {
  Visit(ret.value);
  output << INDENT << "ret" << std::endl;
}

void CodeGenUnit::Visit(const koopa_raw_integer_t &num) {
  output << INDENT << "li a0, " << num.value << std::endl;
}
