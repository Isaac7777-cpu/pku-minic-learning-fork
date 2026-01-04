#pragma once

#include "c_ast.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace koopa_ast {

enum class TypeKind {
  I32,
  Unit,
};

class Base {
public:
  virtual ~Base() = default;

  virtual void Dump(std::ostream &out) const = 0;
};

class Type : public Base {
public:
  Type(TypeKind type_kind) : type(type_kind) {}

  static Type I32() { return {TypeKind::I32}; }
  static Type Unit() { return {TypeKind::Unit}; }

  void Dump(std::ostream &out) const override;

private:
  TypeKind type;
};

enum class ValueKind {
  Integer,
  Return,
};

class Value : public Base {
public:
  virtual ValueKind kind() const = 0;
};

class Return final : public Value {
public:
  ValueKind kind() const override { return ValueKind::Return; }
  std::unique_ptr<Value> return_val;

  void Dump(std::ostream &out) const override;
};

class Integer final : public Value {

public:
  Integer(std::int32_t val) : val_(val) {}
  ValueKind kind() const override { return ValueKind::Integer; }

  void Dump(std::ostream &out) const override;

private:
  std::int32_t val_;
};

class BasicBlock : public Base {
public:
  std::vector<std::unique_ptr<Value>> values;

  BasicBlock(std::string name_) : name(name_) {}
  BasicBlock() : name("") {}
  void Dump(std::ostream &out) const override;

private:
  std::string name;
};

class Function : public Base {
public:
  std::string name;
  std::unique_ptr<Type> type;
  std::vector<std::unique_ptr<BasicBlock>> basicblocks;

  void Dump(std::ostream &out) const override;
};

class Program : public Base {
public:
  std::vector<std::unique_ptr<Value>> global_values;
  std::vector<std::unique_ptr<Function>> functions;

  void Dump(std::ostream &out) const override;
};
} // namespace koopa_ast

std::unique_ptr<koopa_ast::Program>
convert_to_custom_koopa_from_c_reps(std::unique_ptr<c_ast::BaseAST> ast);
