/*******************************************************************************
 *  Function Definition for translation functions.                             *
 ******************************************************************************/

#include "frontend/c_ast.hpp"
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
koopa_ast::Integer *translate_number_c_ast(const c_ast::NumberAST &,
                                           koopa_ast::BasicBlock &);
koopa_ast::Value *translate_unary_exp_c_ast(const c_ast::UnaryExpAST &,
                                            koopa_ast::BasicBlock &);
koopa_ast::Value *translate_mul_exp_c_ast(const c_ast::MulExpAST &,
                                          koopa_ast::BasicBlock &);
koopa_ast::Value *translate_add_exp_c_ast(const c_ast::AddExpAST &,
                                          koopa_ast::BasicBlock &);
koopa_ast::Value *translate_rel_exp_c_ast(const c_ast::RelExpAST &,
                                          koopa_ast::BasicBlock &);
koopa_ast::Value *translate_eq_exp_c_ast(const c_ast::EqExpAST &,
                                         koopa_ast::BasicBlock &);
koopa_ast::Value *translate_land_exp_c_ast(const c_ast::LAndExpAST &,
                                           koopa_ast::BasicBlock &);
koopa_ast::Value *translate_lor_exp_c_ast(const c_ast::LOrExpAST &,
                                          koopa_ast::BasicBlock &);

/*******************************************************************************
 *  Implementation Details for going from each C AST nodes to Koopa Node.      *
 *******************************************************************************/

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

  // TODO: Parse with the new tree that support variable / constant declaration.
  // auto *stmt = dynamic_cast<const c_ast::StmtAST *>(block.stmt.get());
  // if (!stmt)
  //   throw std::runtime_error(
  //       "ir_builder error: BlockAST expects to have StmtAST at param `stmt`");

  // translate_stmt_c_ast(*stmt, *ret.get());

  // return ret;
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
 * Converting a Exp C AST to koopa
 */
koopa_ast::Value *translate_exp_c_ast(const c_ast::ExpAST &exp,
                                      koopa_ast::BasicBlock &block) {
  auto *lor_exp = dynamic_cast<const c_ast::LOrExpAST *>(exp.lor_exp.get());
  if (!lor_exp)
    throw std::runtime_error(
        "ir_builder error: ExpAST expects AddExpAST at param `add_exp`");
  return translate_lor_exp_c_ast(*lor_exp, block);
}

/**
 * Converting a PrimaryExp in C AST to koopa
 */
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
        "implementation: {PrimaryASTExp, PrimaryASTNumber}");
  }
}

/**
 * Going from NumberAST to koopa Integer
 */
// std::unique_ptr<koopa_ast::Integer>
koopa_ast::Integer *translate_number_c_ast(const c_ast::NumberAST &number,
                                           koopa_ast::BasicBlock &block) {
  auto *ret = block.Make<koopa_ast::Integer>(false, number.int_val);
  return ret;
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
        "implementation: {UnaryExpASTPrimary, UnaryExpASTOpUnary}");
  }
}

koopa_ast::Value *translate_mul_exp_c_ast(const c_ast::MulExpAST &mul,
                                          koopa_ast::BasicBlock &block) {

  if (auto *p = dynamic_cast<const c_ast::MulExpASTUnary *>(&mul)) {
    auto *unary = dynamic_cast<const c_ast::UnaryExpAST *>(p->unary_exp.get());
    if (!unary)
      throw std::runtime_error("ir_builder error: MulExpASTUnary expects "
                               "UnaryAST at param `unary_exp`");
    return translate_unary_exp_c_ast(*unary, block);
  } else if (auto *p = dynamic_cast<const c_ast::MulExpASTMulUnary *>(&mul)) {

    // Obtain the sub-node
    auto *mul_exp = dynamic_cast<const c_ast::MulExpAST *>(p->mul_exp.get());
    if (!mul_exp)
      throw std::runtime_error("ir_builder error: MulExpASTUnary expects "
                               "MulExpAST at param `mul_exp`");
    auto *unary_exp =
        dynamic_cast<const c_ast::UnaryExpAST *>(p->unary_exp.get());
    if (!unary_exp)
      throw std::runtime_error("ir_builder error: MulExpASTMulUnary expects "
                               "UnaryAST at param `unary_exp`");

    // Recursively parse the tree
    auto *mul_exp_ast = translate_mul_exp_c_ast(*mul_exp, block);
    auto *unary_exp_ast = translate_unary_exp_c_ast(*unary_exp, block);

    // Build the instruction in koopa
    switch (p->mul_op) {
    case c_ast::MulOp::STAR: {
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Mul,
                                           mul_exp_ast, unary_exp_ast);
    }
    case c_ast::MulOp::SLASH: {
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Div,
                                           mul_exp_ast, unary_exp_ast);
    }
    case c_ast::MulOp::PERCENT: {
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Mod,
                                           mul_exp_ast, unary_exp_ast);
    }
    }
  } else {
    throw std::runtime_error(
        "ir_builder error: MulExpAST must have one of the following "
        "implementation: {MulExpASTUnary, MulExpASTMulUnary}");
  }
}

koopa_ast::Value *translate_add_exp_c_ast(const c_ast::AddExpAST &add,
                                          koopa_ast::BasicBlock &block) {
  if (auto *p = dynamic_cast<const c_ast::AddExpASTMul *>(&add)) {
    auto *mul_exp = dynamic_cast<const c_ast::MulExpAST *>(p->mul_exp.get());
    if (!mul_exp)
      throw std::runtime_error("ir_builder error: AddExpASTMul expects "
                               "MulExpAST at param `mul_exp`");

    return translate_mul_exp_c_ast(*mul_exp, block);
  } else if (auto *p = dynamic_cast<const c_ast::AddExpASTAddMul *>(&add)) {
    // Obtain the sub-node
    auto *add_exp = dynamic_cast<const c_ast::AddExpAST *>(p->add_exp.get());
    if (!add_exp)
      throw std::runtime_error("ir_builder error: AddExpASTAddMul expects "
                               "AddExpAST at param `add_exp`");
    auto *mul_exp = dynamic_cast<const c_ast::MulExpAST *>(p->mul_exp.get());
    if (!mul_exp)
      throw std::runtime_error("ir_builder error: AddExpASTAddMul expects "
                               "MulExpAST at param `mul_exp`");

    // Recursively parse the tree
    auto *add_exp_ast = translate_add_exp_c_ast(*add_exp, block);
    auto *mul_exp_ast = translate_mul_exp_c_ast(*mul_exp, block);

    // Build the instruction in koopa
    switch (p->add_op) {
    case c_ast::AddOp::PLUS:
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Add,
                                           add_exp_ast, mul_exp_ast);
    case c_ast::AddOp::MINUS:
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Sub,
                                           add_exp_ast, mul_exp_ast);
    }
  } else {
    throw std::runtime_error(
        "ir_builder error: AddExpAST must have one of the following "
        "implementation: {AddExpASTMul, AddExpASTAddMul}");
  }
}

koopa_ast::Value *translate_rel_exp_c_ast(const c_ast::RelExpAST &rel,
                                          koopa_ast::BasicBlock &block) {
  if (auto *p = dynamic_cast<const c_ast::RelExpASTAdd *>(&rel)) {
    auto *add_exp = dynamic_cast<const c_ast::AddExpAST *>(p->add_exp.get());
    if (!add_exp) {
      throw std::runtime_error("ir_builder error: RelExpASTAdd expects "
                               "AddExpAST at param `add_exp`");
    }
    return translate_add_exp_c_ast(*add_exp, block);
  } else if (auto *p = dynamic_cast<const c_ast::RelExpASTRelOpAdd *>(&rel)) {
    // Obtain the subnode
    auto *rel_exp = dynamic_cast<const c_ast::RelExpAST *>(p->rel_exp.get());
    if (!rel_exp) {
      throw std::runtime_error("ir_builder error: RelExpASTRelOpAdd expects "
                               "RelExpAST at param `rel_exp`");
    }
    auto *add_exp = dynamic_cast<const c_ast::AddExpAST *>(p->add_exp.get());
    if (!add_exp) {
      throw std::runtime_error("ir_builder error: RelExpASTRelOpAdd expects "
                               "AddExpAST at param `add_exp`");
    }

    // Recursively parse the tree
    auto *rel_exp_ast = translate_rel_exp_c_ast(*rel_exp, block);
    auto *add_exp_ast = translate_add_exp_c_ast(*add_exp, block);

    switch (p->rel_op) {
    case c_ast::RelOp::LT:
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Lt,
                                           rel_exp_ast, add_exp_ast);
    case c_ast::RelOp::LE:
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Le,
                                           rel_exp_ast, add_exp_ast);
    case c_ast::RelOp::GT:
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Gt,
                                           rel_exp_ast, add_exp_ast);
    case c_ast::RelOp::GE:
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Ge,
                                           rel_exp_ast, add_exp_ast);
    }
  } else {
    throw std::runtime_error(
        "ir_builder error: RelExpAST must have one of the following "
        "implementation: {RelExpASTAdd, RelExpASTRelOpAdd}");
  }
}

koopa_ast::Value *translate_eq_exp_c_ast(const c_ast::EqExpAST &eq,
                                         koopa_ast::BasicBlock &block) {
  if (auto *p = dynamic_cast<const c_ast::EqExpASTRel *>(&eq)) {
    auto *rel_exp = dynamic_cast<const c_ast::RelExpAST *>(p->rel_exp.get());
    if (!rel_exp) {
      throw std::runtime_error("ir_builder error: EqExpASTRel expects "
                               "RelExpAST at param `rel_exp`");
    }
    return translate_rel_exp_c_ast(*rel_exp, block);
  } else if (auto *p = dynamic_cast<const c_ast::EqExpASTEqOpRel *>(&eq)) {
    // Obtain the subnode
    auto *eq_exp = dynamic_cast<const c_ast::EqExpAST *>(p->eq_exp.get());
    if (!eq_exp) {
      throw std::runtime_error("ir_builder error: RelExpASTRelOpAdd expects "
                               "RelExpAST at param `rel_exp`");
    }
    auto *rel_exp = dynamic_cast<const c_ast::RelExpAST *>(p->rel_exp.get());
    if (!rel_exp) {
      throw std::runtime_error("ir_builder error: RelExpASTRelOpAdd expects "
                               "RelExpAST at param `rel_exp`");
    }

    // Recursively parse the tree
    auto *eq_exp_ast = translate_eq_exp_c_ast(*eq_exp, block);
    auto *rel_exp_ast = translate_rel_exp_c_ast(*rel_exp, block);

    switch (p->eq_op) {
    case c_ast::EqOp::EQ:
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Eq,
                                           eq_exp_ast, rel_exp_ast);
    case c_ast::EqOp::NE:
      return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::NotEq,
                                           eq_exp_ast, rel_exp_ast);
    }
  } else {
    throw std::runtime_error(
        "ir_builder error: EqExpAST must have one of the following "
        "implementation: {EqExpASTRel, EqExpASTRelOpAdd}");
  }
}

koopa_ast::Value *translate_land_exp_c_ast(const c_ast::LAndExpAST &land,
                                           koopa_ast::BasicBlock &block) {
  if (auto *p = dynamic_cast<const c_ast::LAndExpASTEq *>(&land)) {
    auto *eq_exp = dynamic_cast<const c_ast::EqExpAST *>(p->eq_exp.get());
    if (!eq_exp) {
      throw std::runtime_error(
          "ir_builder error: LAndExpASTEq expects EqExpAST at param `eq_exp`");
    }
    return translate_eq_exp_c_ast(*eq_exp, block);
  } else if (auto *p = dynamic_cast<const c_ast::LAndExpASTLAndEq *>(&land)) {
    auto *land_exp = dynamic_cast<const c_ast::LAndExpAST *>(p->land_exp.get());
    if (!land_exp) {
      throw std::runtime_error("ir_builder error: LAndExpASTLAndEq expects "
                               "LAndExpAST at param `land_exp`");
    }
    auto *eq_exp = dynamic_cast<const c_ast::EqExpAST *>(p->eq_exp.get());
    if (!eq_exp) {
      throw std::runtime_error("ir_builder error: LAndExpASTLAndEq expects "
                               "EqExpAST at param `eq_exp`");
    }

    // Recursively parsing
    auto *land_exp_ast = translate_land_exp_c_ast(*land_exp, block);
    auto *eq_exp_ast = translate_eq_exp_c_ast(*eq_exp, block);

    // Build the instruction in koopa
    auto *zero = block.Make<koopa_ast::Integer>(false, 0);
    auto *left_bool = block.Make<koopa_ast::Binary>(
        true, koopa_ast::BinaryOp::NotEq, land_exp_ast, zero);
    auto *right_bool = block.Make<koopa_ast::Binary>(
        true, koopa_ast::BinaryOp::NotEq, eq_exp_ast, zero);
    return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::And,
                                         left_bool, right_bool);
  } else {
    throw std::runtime_error(
        "ir_builder error: LAndExpAST must have one of the following "
        "implementation: {LAndExpASTEq, LAndExpASTLAndEq}");
  }
}

koopa_ast::Value *translate_lor_exp_c_ast(const c_ast::LOrExpAST &lor,
                                          koopa_ast::BasicBlock &block) {
  if (auto *p = dynamic_cast<const c_ast::LOrExpASTLAnd *>(&lor)) {
    auto *land_exp = dynamic_cast<const c_ast::LAndExpAST *>(p->land_exp.get());
    if (!land_exp) {
      throw std::runtime_error("ir_builder error: LOrExpASTLAnd expects "
                               "LAndExpAST at param `land_exp`");
    }
    return translate_land_exp_c_ast(*land_exp, block);
  } else if (auto *p = dynamic_cast<const c_ast::LOrExpASTLOrLAnd *>(&lor)) {
    auto *lor_exp = dynamic_cast<const c_ast::LOrExpAST *>(p->lor_exp.get());
    if (!lor_exp) {
      throw std::runtime_error("ir_builder error: LOrExpASTLOrLAnd expects "
                               "LOrExpAST at param `lor_exp`");
    }
    auto *land_exp = dynamic_cast<const c_ast::LAndExpAST *>(p->land_exp.get());
    if (!land_exp) {
      throw std::runtime_error("ir_builder error: LOrExpASTLOrLAnd expects "
                               "LAndExpAST at param `land_exp`");
    }

    // Recursive parse
    auto *lor_exp_ast = translate_lor_exp_c_ast(*lor_exp, block);
    auto *land_exp_ast = translate_land_exp_c_ast(*land_exp, block);

    // Build the instruction
    auto *zero = block.Make<koopa_ast::Integer>(false, 0);
    auto *left_bool = block.Make<koopa_ast::Binary>(
        true, koopa_ast::BinaryOp::NotEq, lor_exp_ast, zero);
    auto *right_bool = block.Make<koopa_ast::Binary>(
        true, koopa_ast::BinaryOp::NotEq, land_exp_ast, zero);
    return block.Make<koopa_ast::Binary>(true, koopa_ast::BinaryOp::Or,
                                         left_bool, right_bool);
  } else {
    throw std::runtime_error(
        "ir_builder error: LOrExpAST must have one of the following "
        "implementation: {LOrExpASTLAnd, LOrExpASTLOrLAnd}");
  }
}

/*******************************************************************************
 *  Public API Implementation for translation.                                 *
 *******************************************************************************/

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
