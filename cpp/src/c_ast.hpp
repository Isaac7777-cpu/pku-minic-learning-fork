#pragma once

#include <iostream>
#include <memory>

namespace c_ast {

enum class UnaryOp { PLUS, MINUS, BANG, TILDE };

inline const char *ToString(UnaryOp op) {
  switch (op) {
  case UnaryOp::PLUS:
    return "+";
  case UnaryOp::MINUS:
    return "-";
  case UnaryOp::BANG:
    return "!";
  case UnaryOp::TILDE:
    return "~";
  }
}

class BaseAST {
public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
};

class CompUnitAST final : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override {
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << "}";
  }
};

class FuncDefAST final : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump() const override {
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    block->Dump();
    std::cout << " }";
  }
};

class FuncTypeAST : public BaseAST {
public:
  void Dump() const override {
    std::cout << "FuncTypeAST { ";
    std::cout << "int";
    std::cout << " }";
  }
};

class BlockAST final : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override {
    std::cout << "BlockAST { ";
    stmt->Dump();
    std::cout << " }";
  }
};

class StmtAST final : public BaseAST {
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override {
    std::cout << "StmtAST { ";
    exp->Dump();
    std::cout << " }";
  }
};

class ExpAST final : public BaseAST {
public:
  std::unique_ptr<BaseAST> unary_exp;

  void Dump() const override {
    std::cout << "ExpAST { ";
    unary_exp->Dump();
    std::cout << " }";
  }
};

class PrimaryAST : public BaseAST {
public:
  virtual ~PrimaryAST() = default;
};

class PrimaryASTExp final : public PrimaryAST {
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override {
    std::cout << "PrimaryAST { ";
    exp->Dump();
    std::cout << " }";
  }
};

class PrimaryASTNumber final : public PrimaryAST {
public:
  std::unique_ptr<BaseAST> number;

  void Dump() const override {
    std::cout << "PrimaryAST { ";
    number->Dump();
    std::cout << " }";
  }
};

class NumberAST final : public BaseAST {
public:
  int int_val;

  void Dump() const override { std::cout << "Number { " << int_val << " }"; }
};

class UnaryExpAST : public BaseAST {
public:
  virtual ~UnaryExpAST() = default;
};

class UnaryExpASTPrimary final : public UnaryExpAST {
public:
  std::unique_ptr<BaseAST> primary_exp;

  void Dump() const override {
    std::cout << "UnaryExpAST { ";
    primary_exp->Dump();
    std::cout << " }";
  }
};

class UnaryExpASTOpUnary final : public UnaryExpAST {
public:
  UnaryOp unary_op;
  std::unique_ptr<BaseAST> unary_exp;

  void Dump() const override {
    std::cout << "UnaryExpAST { " << ToString(unary_op) << "( ";
    unary_exp->Dump();
    std::cout << " ) }";
  }
};

} // namespace c_ast
