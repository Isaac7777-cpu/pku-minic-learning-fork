#pragma once

#include <iostream>
#include <memory>

namespace c_ast {

enum class UnaryOp { PLUS, MINUS, BANG, TILDE };
enum class MulOp { STAR, SLASH, PERCENT };
enum class AddOp { PLUS, MINUS };
enum class RelOp { LT, LE, GT, GE };
enum class EqOp { EQ, NE };

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

inline const char *ToString(MulOp op) {
  switch (op) {
  case MulOp::STAR:
    return "*";
  case MulOp::SLASH:
    return "/";
  case MulOp::PERCENT:
    return "%";
  }
}

inline const char *ToString(AddOp op) {
  switch (op) {
  case AddOp::PLUS:
    return "+";
  case AddOp::MINUS:
    return "-";
  }
}

inline const char *ToString(RelOp op) {
  switch (op) {
  case RelOp::LT:
    return "<";
  case RelOp::LE:
    return "<=";
  case RelOp::GT:
    return ">";
  case RelOp::GE:
    return ">=";
  }
}

inline const char *ToString(EqOp op) {
  switch (op) {
  case EqOp::EQ:
    return "==";
  case EqOp::NE:
    return "!=";
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
    std::cout << " }";
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
  std::unique_ptr<BaseAST> lor_exp;

  void Dump() const override {
    std::cout << "ExpAST { ";
    lor_exp->Dump();
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

class MulExpAST : public BaseAST {
public:
  virtual ~MulExpAST() = default;
};

class MulExpASTUnary final : public MulExpAST {
public:
  std::unique_ptr<BaseAST> unary_exp;

  void Dump() const override {
    std::cout << "MulExpAST { ";
    unary_exp->Dump();
    std::cout << " }";
  }
};

class MulExpASTMulUnary final : public MulExpAST {
public:
  MulOp mul_op;
  std::unique_ptr<BaseAST> mul_exp;
  std::unique_ptr<BaseAST> unary_exp;

  void Dump() const override {
    std::cout << "MulExpAST { " << ToString(mul_op) << "( ";
    mul_exp->Dump();
    std::cout << " , ";
    unary_exp->Dump();
    std::cout << " ) }";
  }
};

class AddExpAST : public BaseAST {
public:
  virtual ~AddExpAST() = default;
};

class AddExpASTMul final : public AddExpAST {
public:
  std::unique_ptr<BaseAST> mul_exp;

  void Dump() const override {
    std::cout << "AddExpAST { ";
    mul_exp->Dump();
    std::cout << " }";
  }
};

class AddExpASTAddMul final : public AddExpAST {
public:
  AddOp add_op;
  std::unique_ptr<BaseAST> add_exp;
  std::unique_ptr<BaseAST> mul_exp;

  void Dump() const override {
    std::cout << "AddExpAST { " << ToString(add_op) << "( ";
    add_exp->Dump();
    std::cout << " , ";
    mul_exp->Dump();
    std::cout << " ) }";
  }
};

class RelExpAST : public BaseAST {
public:
  virtual ~RelExpAST() = default;
};

class RelExpASTAdd final : public RelExpAST {
public:
  std::unique_ptr<BaseAST> add_exp;

  void Dump() const override {
    std::cout << "RelExpAST { ";
    add_exp->Dump();
    std::cout << " }";
  }
};

class RelExpASTRelOpAdd final : public RelExpAST {
public:
  std::unique_ptr<BaseAST> rel_exp;
  RelOp rel_op;
  std::unique_ptr<BaseAST> add_exp;

  void Dump() const override {
    std::cout << "RelExpAST { " << ToString(this->rel_op) << "( ";
    this->rel_exp->Dump();
    std::cout << " , ";
    this->add_exp->Dump();
    std::cout << " ) }";
  }
};

class EqExpAST : public BaseAST {
public:
  virtual ~EqExpAST() = default;
};

class EqExpASTRel final : public EqExpAST {
public:
  std::unique_ptr<BaseAST> rel_exp;

  void Dump() const override {
    std::cout << "EqExpAST { ";
    this->rel_exp->Dump();
    std::cout << " }";
  }
};

class EqExpASTEqOpRel final : public EqExpAST {
public:
  std::unique_ptr<BaseAST> eq_exp;
  EqOp eq_op;
  std::unique_ptr<BaseAST> rel_exp;

  void Dump() const override {
    std::cout << "EqExpAST { " << ToString(eq_op) << "( ";
    this->eq_exp->Dump();
    std::cout << " , ";
    this->rel_exp->Dump();
    std::cout << " ) }";
  }
};

class LAndExpAST : public BaseAST {
public:
  virtual ~LAndExpAST() = default;
};

class LAndExpASTEq final : public LAndExpAST {
public:
  std::unique_ptr<BaseAST> eq_exp;

  void Dump() const override {
    std::cout << "LAndExpAST { ";
    this->eq_exp->Dump();
    std::cout << " }";
  }
};

class LAndExpASTLAndEq final : public LAndExpAST {
public:
  std::unique_ptr<BaseAST> land_exp;
  std::unique_ptr<BaseAST> eq_exp;

  void Dump() const override {
    std::cout << "LAndExpAST { &&( ";
    this->land_exp->Dump();
    std::cout << " , ";
    this->eq_exp->Dump();
    std::cout << " ) }";
  };
};

class LOrExpAST : public BaseAST {
public:
  virtual ~LOrExpAST() = default;
};

class LOrExpASTLAnd final : public LOrExpAST {
public:
  std::unique_ptr<BaseAST> land_exp;

  void Dump() const override {
    std::cout << "LOrExpAST { ";
    this->land_exp->Dump();
    std::cout << " }";
  }
};

class LOrExpASTLOrLAnd final : public LOrExpAST {
public:
  std::unique_ptr<BaseAST> lor_exp;
  std::unique_ptr<BaseAST> land_exp;

  void Dump() const override {
    std::cout << "LOrExpAST { ||( ";
    this->lor_exp->Dump();
    std::cout << " , ";
    this->land_exp->Dump();
    std::cout << " ) }";
  }
};

} // namespace c_ast
