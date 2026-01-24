#pragma once

#include "koopa.h"
#include <cstdint>
#include <memory>
#include <optional>
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

  virtual void Dump(std::ostream &out) = 0;
};

class Type : public Base {
public:
  Type(TypeKind type_kind) : type(type_kind) {}

  static Type I32() { return {TypeKind::I32}; }
  static Type Unit() { return {TypeKind::Unit}; }

  void Dump(std::ostream &out) override;

private:
  TypeKind type;
};

enum class ValueKind { Integer, Return, Binary };
enum class BinaryOp {
  NotEq,
  Eq,
  Gt,
  Lt,
  Ge,
  Le,
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  And,
  Or,
  Xor,
  Shl,
  Shr,
  Sar
};

std::string get_binary_op_repr(const BinaryOp &);

class Value : public Base {
public:
  std::optional<std::string> name;
  virtual ValueKind kind() const = 0;
  virtual std::string get_reprs() = 0;
};

class Integer final : public Value {
private:
  std::int32_t val_;

public:
  Integer(std::int32_t val) : val_(val) {}
  ValueKind kind() const override { return ValueKind::Integer; }
  void Dump(std::ostream &out) override;
  std::string get_reprs() override;
};

class Return final : public Value {
private:
  Value *return_val;

public:
  Return(Value *ret_val_) : return_val(ret_val_) {}
  ValueKind kind() const override { return ValueKind::Return; }
  void Dump(std::ostream &out) override;
  std::string get_reprs() override;
};

class Binary final : public Value {
private:
  Value *lhs;
  Value *rhs;
  BinaryOp op;

public:
  Binary(BinaryOp op_, Value *lhs_, Value *rhs_)
      : lhs(lhs_), rhs(rhs_), op(op_) {}
  ValueKind kind() const override { return ValueKind::Binary; }
  void Dump(std::ostream &out) override;
  std::string get_reprs() override;
};

class BasicBlock : public Base {
public:
  std::vector<std::unique_ptr<Value>> pool;
  std::vector<Value *> insts;

  BasicBlock(std::string name_) : name(name_) {}
  BasicBlock() : name("") {}

  // allocates in pool, optionally also appends to insts
  template <class T, class... Args> T *Make(bool is_inst, Args &&...args) {
    pool.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    auto *p = static_cast<T *>(pool.back().get());
    if (is_inst)
      insts.push_back(p);
    return p;
  }

  void Dump(std::ostream &out) override;

private:
  std::string name;
};

class Function : public Base {
public:
  std::string name;
  std::unique_ptr<Type> type;
  std::vector<std::unique_ptr<BasicBlock>> basicblocks;

  void Dump(std::ostream &out) override;
};

class Program : public Base {
public:
  std::vector<std::unique_ptr<Value>> global_values;
  std::vector<std::unique_ptr<Function>> functions;

  void Dump(std::ostream &out) override;
};
} // namespace koopa_ast
