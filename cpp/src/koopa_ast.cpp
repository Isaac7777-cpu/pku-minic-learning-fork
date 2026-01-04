#include "koopa_ast.hpp"
#include "c_ast.hpp"

#include <cassert>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>

static constexpr const std::string_view INDENT = "  ";

namespace koopa_ast {

void Type::Dump(std::ostream &out) const {

  switch (type) {
  case koopa_ast::TypeKind::I32:
    out << "i32";
    break;
  case koopa_ast::TypeKind::Unit:
    out << "void";
    break;
  }
}

void Integer::Dump(std::ostream &out) const { out << val_; }

void Return::Dump(std::ostream &out) const {
  out << INDENT << "ret ";
  return_val->Dump(out);
  out << "\n";
}

void BasicBlock::Dump(std::ostream &out) const {
  if (!name.empty()) {
    out << name << ":\n";
  }

  for (auto const &v : values) {
    if (v)
      v->Dump(out);
  }
}

void Function::Dump(std::ostream &out) const {
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

void Program::Dump(std::ostream &out) const {
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

/*************************************************************************
 * Implementation Details for going from each C AST nodes to Koopa Node. *
 *************************************************************************/

/**
 * Going from NumberAST to koopa Integer
 */
std::unique_ptr<koopa_ast::Integer>
translate_number_c_ast(const c_ast::NumberAST &number) {
  return std::make_unique<koopa_ast::Integer>(number.int_val);
}

/**
 * Going from StmtAST to koopa stmt
 *
 * TODO: Currently, StmtAST is only the return value.
 */
std::unique_ptr<koopa_ast::Return>
translate_stmt_c_ast(const c_ast::StmtAST &stmt) {
  auto ret = std::make_unique<koopa_ast::Return>();

  auto *num = dynamic_cast<const c_ast::NumberAST *>(stmt.number.get());
  if (!num) {
    throw std::runtime_error("StmtAST expects NumberAST");
  }

  ret->return_val = translate_number_c_ast(*num);
  return ret;
}

/**
 * Converting FuncType C AST to just Type koopa IR reps.
 *
 * HACK: Currently since we don't have type for the c_ast,
 * this function will always return i32.
 */
std::unique_ptr<koopa_ast::Type>
translate_func_type_c_ast(const c_ast::FuncTypeAST &func_type) {
  // HACK: Currently it is just hard matching as it has to be int
  return std::make_unique<koopa_ast::Type>(koopa_ast::Type::I32());
}

/**
 * Converting a Block C AST to koopa Block for now.
 *
 * NOTE: The meaning of a block in C and a block in koopa is different.
 */
std::unique_ptr<koopa_ast::BasicBlock>
translate_block_c_ast(const c_ast::BlockAST &block, std::string name = "") {
  auto ret = std::make_unique<koopa_ast::BasicBlock>(name);

  auto *stmt = dynamic_cast<const c_ast::StmtAST *>(block.stmt.get());
  if (!stmt) {
    throw std::runtime_error("BlockAST expects to have StmtAST");
  }

  ret->values.push_back(translate_stmt_c_ast(*stmt));

  return ret;
}

/**
 * Converting a FuncDefAST to a koopa function as the following:
 *
 *
 * ```
 *
 *  c_ast::FuncDefAST:name (string)
 *    -> koopa_ast::Function:name (string)
 *
 *  c_ast::FuncDefAST:type (string)
 *    -> koopa_ast::Function:type (koopa_ast::Type)
 *
 *  c_ast::FuncDefAST:block (c_ast::BlockAST)
 *    -> koopa_ast::Function:basicblocks (vector<koopa_ast::BasicBlock>)
 *
 * ```
 *
 *
 * NOTE: I think this does satisfy the definition of a koopa block as it does
 * only have one point of entry and only leave or return. However, this may not
 * be the most optimised way to write this, of course.
 */
std::unique_ptr<koopa_ast::Function>
translate_func_def_c_ast(const c_ast::FuncDefAST &func_def) {
  auto ret = std::make_unique<koopa_ast::Function>();

  // Get the function name
  ret->name = "@" + func_def.ident;

  // Get the type
  auto *type =
      dynamic_cast<const c_ast::FuncTypeAST *>(func_def.func_type.get());
  if (!type) {
    throw std::runtime_error(
        "FuncDefAST expects FuncTypeAST at param `func_type`");
  }

  ret->type = translate_func_type_c_ast(*type);

  // Get the block
  auto *block = dynamic_cast<const c_ast::BlockAST *>(func_def.block.get());
  if (!block) {
    throw std::runtime_error("FuncDefAST expects BlockAST at param `block`");
  }

  // HACK: We only have one block for now.
  ret->basicblocks.push_back(translate_block_c_ast(*block, "%entry"));

  return ret;
}

/**
 * Converting a CompUnitAST in C to a program in koopa.
 */
std::unique_ptr<koopa_ast::Program>
translate_comp_unit_c_ast(const c_ast::CompUnitAST &comp_unit) {
  auto ret = std::make_unique<koopa_ast::Program>();

  auto *func_def =
      dynamic_cast<const c_ast::FuncDefAST *>(comp_unit.func_def.get());
  if (!func_def) {
    throw std::runtime_error(
        "CompUnitAST expects FuncDefAST at param `func_def`");
  }

  ret->functions.push_back(translate_func_def_c_ast(*func_def));

  return ret;
}

/*
 * Exposed API for converting from C AST to Koopa Representation
 */
std::unique_ptr<koopa_ast::Program>
convert_to_custom_koopa_from_c_reps(std::unique_ptr<c_ast::BaseAST> ast) {
  auto *comp_unit = dynamic_cast<const c_ast::CompUnitAST *>(ast.get());

  if (!comp_unit) {
    throw std::runtime_error(
        "Expects to have a C AST with root of CompUnitAST");
  }

  return translate_comp_unit_c_ast(*comp_unit);
}
