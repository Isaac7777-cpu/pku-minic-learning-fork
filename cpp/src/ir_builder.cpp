/*******************************************************************************
 *  Function Definition for translation functions.                             *
 ******************************************************************************/

#include "c_ast.hpp"
#include "koopa_ast.hpp"
#include <stdexcept>

std::unique_ptr<koopa_ast::Program>
translate_comp_unit_c_ast(const c_ast::CompUnitAST &);
std::unique_ptr<koopa_ast::Function>
translate_func_def_c_ast(const c_ast::FuncDefAST &);
std::unique_ptr<koopa_ast::Type>
translate_func_type_c_ast(const c_ast::FuncTypeAST &);
std::unique_ptr<koopa_ast::BasicBlock>
translate_block_c_ast(const c_ast::BlockAST &block, std::string = "");
koopa_ast::Value *translate_stmt_c_ast(const c_ast::StmtAST &,
                                       koopa_ast::BasicBlock &);
koopa_ast::Value *translate_exp_c_ast(const c_ast::ExpAST &,
                                      koopa_ast::BasicBlock &);
koopa_ast::Value *translate_primary_exp_c_ast(const c_ast::PrimaryAST &,
                                              koopa_ast::BasicBlock &);
koopa_ast::Value *translate_unary_exp_c_ast(const c_ast::UnaryExpAST &,
                                            koopa_ast::BasicBlock &);
koopa_ast::Integer *translate_number_c_ast(const c_ast::NumberAST &,
                                           koopa_ast::BasicBlock &);

/*******************************************************************************
 *  Implementation Details for going from each C AST nodes to Koopa Node.      *
 *******************************************************************************/

/**
 * Going from NumberAST to koopa Integer
 */
// std::unique_ptr<koopa_ast::Integer>
koopa_ast::Integer *translate_number_c_ast(const c_ast::NumberAST &number,
                                           koopa_ast::BasicBlock &block) {
  auto *ret = block.Make<koopa_ast::Integer>(false, number.int_val);
  return ret;
}

koopa_ast::Value *translate_primary_exp_c_ast(const c_ast::PrimaryAST &primary,
                                              koopa_ast::BasicBlock &block) {
  if (auto *p = dynamic_cast<const c_ast::PrimaryASTExp *>(&primary)) {
    auto *exp = dynamic_cast<const c_ast::ExpAST *>(p->exp.get());
    if (!exp)
      throw std::runtime_error("ir_builder error: PrimaryASTExp expects "
                               "ExpAST at param `exp`");

    return translate_exp_c_ast(*exp, block);
  } else if (auto *p_num =
                 dynamic_cast<const c_ast::PrimaryASTNumber *>(&primary)) {
    auto *num = dynamic_cast<const c_ast::NumberAST *>(p_num->number.get());
    if (!num)
      throw std::runtime_error(
          "ir_builder error: PrimaryASTNumber expects NumberAST at param "
          "`number`");
    return block.Make<koopa_ast::Integer>(false, num->int_val);
  } else {
    throw std::runtime_error(
        "ir_builder error: PrimaryAST must have one of the following "
        "implementation: {PrimaryASTExp, PrimaryASTNumber} ");
  }
}

koopa_ast::Value *translate_unary_exp_c_ast(const c_ast::UnaryExpAST &unary,
                                            koopa_ast::BasicBlock &block) {

  if (auto *p = dynamic_cast<const c_ast::UnaryExpASTPrimary *>(&unary)) {

    auto *prim = dynamic_cast<const c_ast::PrimaryAST *>(p->primary_exp.get());
    if (!prim)
      throw std::runtime_error("ir_builder error: UnaryExpASTPrimary expects "
                               "PrimaryAST at param `primary_exp`");
    return translate_primary_exp_c_ast(*prim, block);
  } else if (auto *op =
                 dynamic_cast<const c_ast::UnaryExpASTOpUnary *>(&unary)) {

    auto *u_exp = dynamic_cast<const c_ast::UnaryExpAST *>(op->unary_exp.get());

    if (!u_exp)
      throw std::runtime_error("ir_builder error: UnaryExpASTOpUnary expects "
                               "UnaryExpAST at param `unary_exp`");

    auto *u_exp_ast = translate_unary_exp_c_ast(*u_exp, block);

    // Generate the instruction based on the operation type
    switch (op->unary_op) {
    case c_ast::UnaryOp::PLUS: {
      return u_exp_ast;
      break;
    }
    case c_ast::UnaryOp::MINUS: {
      auto *zero = block.Make<koopa_ast::Integer>(false, 0);
      auto *inst = block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Sub,
                                                 zero, u_exp_ast);
      return inst;
      break;
    }
    case c_ast::UnaryOp::BANG: {
      auto *zero = block.Make<koopa_ast::Integer>(false, 0);
      auto *inst = block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Eq,
                                                 u_exp_ast, zero);
      return inst;
      break;
    }
    case c_ast::UnaryOp::TILDE: {
      auto *all_one = block.Make<koopa_ast::Integer>(false, -1);
      auto *inst = block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Xor,
                                                 u_exp_ast, all_one);
      return inst;
      break;
    }
    }
  } else {
    throw std::runtime_error(
        "ir_builder error: UnaryExpAST must have one of the following "
        "implementation: {UnaryExpASTPrimary, UnaryExpASTOpUnary} ");
  }
}

koopa_ast::Value *translate_exp_c_ast(const c_ast::ExpAST &exp,
                                      koopa_ast::BasicBlock &block) {
  auto *unary_exp =
      dynamic_cast<const c_ast::UnaryExpAST *>(exp.unary_exp.get());
  if (!unary_exp)
    throw std::runtime_error(
        "ir_builder error: ExpAST expects UnaryExpAST at param `unary_exp`");
  return translate_unary_exp_c_ast(*unary_exp, block);
}

/**
 * Going from StmtAST to koopa stmt
 *
 */
koopa_ast::Value *translate_stmt_c_ast(const c_ast::StmtAST &stmt,
                                       koopa_ast::BasicBlock &block) {
  // Cast and check that it has the correct type
  auto *exp = dynamic_cast<const c_ast::ExpAST *>(stmt.exp.get());
  if (!exp)
    throw std::runtime_error(
        "ir_builder error: StmtAST expects ExpAST at param `exp`");

  // Translate the expression inside the return
  auto *val = translate_exp_c_ast(*exp, block);

  return block.Make<koopa_ast::Return>(true, val);
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
translate_block_c_ast(const c_ast::BlockAST &block, std::string name) {
  auto ret = std::make_unique<koopa_ast::BasicBlock>(name);

  auto *stmt = dynamic_cast<const c_ast::StmtAST *>(block.stmt.get());
  if (!stmt)
    throw std::runtime_error(
        "ir_builder error: BlockAST expects to have StmtAST at param `stmt`");

  translate_stmt_c_ast(*stmt, *ret.get());

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
    throw std::runtime_error("ir_builder error: FuncDefAST expects "
                             "FuncTypeAST at param `func_type`");
  }

  ret->type = translate_func_type_c_ast(*type);

  // Get the block
  auto *block = dynamic_cast<const c_ast::BlockAST *>(func_def.block.get());
  if (!block) {
    throw std::runtime_error(
        "ir_builder error: FuncDefAST expects BlockAST at param `block`");
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
        "ir_builder error: CompUnitAST expects FuncDefAST at param `func_def`");
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
        "ir_builder error: Expects to have a C AST with root of CompUnitAST");
  }

  return translate_comp_unit_c_ast(*comp_unit);
}
