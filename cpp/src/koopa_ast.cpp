#include "koopa_ast.hpp"
#include "name_manager.hpp"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

static constexpr const std::string_view INDENT = "\t";

namespace koopa_ast {

std::string get_binary_op_repr(const BinaryOp &op) {
  switch (op) {
  case BinaryOp::NotEq:
    return "ne";
  case BinaryOp::Eq:
    return "eq";
  case BinaryOp::Gt:
    return "gt";
  case BinaryOp::Lt:
    return "lt";
  case BinaryOp::Ge:
    return "ge";
  case BinaryOp::Le:
    return "le";
  case BinaryOp::Add:
    return "add";
  case BinaryOp::Sub:
    return "sub";
  case BinaryOp::Mul:
    return "mul";
  case BinaryOp::Div:
    return "div";
  case BinaryOp::Mod:
    return "mod";
  case BinaryOp::And:
    return "and";
  case BinaryOp::Or:
    return "or";
  case BinaryOp::Xor:
    return "xor";
  case BinaryOp::Shl:
    return "shl";
  case BinaryOp::Shr:
    return "shr";
  case BinaryOp::Sar:
    return "sar";
  }
}

struct ctx {
  VarNameManager name_manager;
} current_ctx;

// Definition of `get_reprs` methods

std::string Integer::get_reprs() {
  this->name = std::to_string(this->val_);
  return *this->name;
}

std::string Return::get_reprs() {
  throw std::runtime_error(
      "Return statement do not have names / representations.");
}

std::string Binary::get_reprs() {
  if (!this->name) {
    this->name = current_ctx.name_manager.get_new_var_name();
  }
  return *this->name;
}

// Definition of Dump methods

void Type::Dump(std::ostream &out) {
  switch (type) {
  case koopa_ast::TypeKind::I32:
    out << "i32";
    break;
  case koopa_ast::TypeKind::Unit:
    out << "void";
    break;
  }
}

void Integer::Dump(std::ostream &out) { out << this->val_; }

void Return::Dump(std::ostream &out) {
  // If we are returning an immediate, then return it directly
  out << INDENT << "ret " << this->return_val->get_reprs() << std::endl;
}

void Binary::Dump(std::ostream &out) {
  out << INDENT << this->get_reprs() << " = " << get_binary_op_repr(this->op)
      << " " << this->lhs->get_reprs() << " " << this->rhs->get_reprs()
      << std::endl;
}

void BasicBlock::Dump(std::ostream &out) {
  if (!this->name.empty()) {
    out << this->name << ":\n";
  }

  for (auto const &v : this->insts) {
    if (v)
      v->Dump(out);
  }
}

void Function::Dump(std::ostream &out) {
  out << "fun " << name << "(): ";
  type->Dump(out);
  out << " {\n";
  for (size_t i = 0; i < basicblocks.size(); ++i) {
    if (basicblocks[i])
      basicblocks[i]->Dump(out);

    if (i + 1 < basicblocks.size())
      out << "\n";
  }
  out << "}\n";
}

void Program::Dump(std::ostream &out) {
  for (auto const &gv : global_values) {
    if (gv)
      gv->Dump(out);
  }
  for (auto const &f : functions) {
    if (f)
      f->Dump(out);
  }
}

} // namespace koopa_ast
