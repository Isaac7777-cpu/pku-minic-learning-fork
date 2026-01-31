#pragma once

#include "codegen_ctx.hpp"
#include "koopa.h"
#include <iostream>
#include <memory>

class IKoopaVisitor {
private:
  virtual void Visit(const koopa_raw_program_t &) = 0;
  virtual void Visit(const koopa_raw_slice_t &) = 0;
  virtual void Visit(const koopa_raw_function_t &) = 0;
  virtual void Visit(const koopa_raw_basic_block_t &) = 0;
  virtual reg_t Visit(const koopa_raw_value_t &) = 0;
  virtual reg_t Visit(const koopa_raw_return_t &) = 0;
  virtual reg_t Visit(const koopa_raw_integer_t &) = 0;
  virtual reg_t Visit(const koopa_raw_binary_t &) = 0;

public:
  virtual ~IKoopaVisitor() = default;
};

class CodeGenUnit : public IKoopaVisitor {
private:
  unsigned int indent_level = 0;
  std::ostream &output;

  std::unique_ptr<CodeGenCtx> ctx;

  void Visit(const koopa_raw_program_t &) override;
  void Visit(const koopa_raw_slice_t &) override;
  void Visit(const koopa_raw_function_t &) override;
  void Visit(const koopa_raw_basic_block_t &) override;
  reg_t Visit(const koopa_raw_value_t &) override;
  reg_t Visit(const koopa_raw_return_t &) override;
  reg_t Visit(const koopa_raw_integer_t &) override;
  reg_t Visit(const koopa_raw_binary_t &) override;

public:
  CodeGenUnit(std::ostream &_dest) : output(_dest) {
    ctx = std::make_unique<CodeGenCtx>();
  }
  void generate(const koopa_raw_program_t &);
};
