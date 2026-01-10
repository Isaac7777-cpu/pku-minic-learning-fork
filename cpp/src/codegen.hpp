#pragma once

#include "koopa.h"
#include <iostream>

class IKoopaVisitor {
private:
  virtual void Visit(const koopa_raw_program_t &) = 0;
  virtual void Visit(const koopa_raw_slice_t &) = 0;
  virtual void Visit(const koopa_raw_function_t &) = 0;
  virtual void Visit(const koopa_raw_basic_block_t &) = 0;
  virtual void Visit(const koopa_raw_value_t &) = 0;
  virtual void Visit(const koopa_raw_return_t &) = 0;
  virtual void Visit(const koopa_raw_integer_t &) = 0;

public:
  virtual ~IKoopaVisitor() = default;
};

class CodeGenUnit : public IKoopaVisitor {
private:
  unsigned int indent_level = 0;
  std::ostream &output;

  void Visit(const koopa_raw_program_t &) override;
  void Visit(const koopa_raw_slice_t &) override;
  void Visit(const koopa_raw_function_t &) override;
  void Visit(const koopa_raw_basic_block_t &) override;
  void Visit(const koopa_raw_value_t &) override;
  void Visit(const koopa_raw_return_t &) override;
  void Visit(const koopa_raw_integer_t &) override;

public:
  CodeGenUnit(std::ostream &_dest) : output(_dest) {}
  void generate(const koopa_raw_program_t &);
};
