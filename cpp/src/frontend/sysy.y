%code requires {
  #include <memory>
  #include <string>
  #include "frontend/c_ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "frontend/c_ast.hpp"

// Declare lexer function and error handling
int yylex();
void yyerror(std::unique_ptr<c_ast::BaseAST> &ast, const char *s);

using namespace std;

%}

%parse-param { std::unique_ptr<c_ast::BaseAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  c_ast::UnaryOp unary_op_val;
  c_ast::MulOp mul_op_val;
  c_ast::AddOp add_op_val;
  c_ast::RelOp rel_op_val;
  c_ast::EqOp eq_op_val;
  c_ast::BaseAST *ast_val;
  std::vector<std::unique_ptr<c_ast::BaseAST>>* vec;
}

%token INT RETURN LE GE EQ NE AND OR CONST
%token <str_val> IDENT
%token <int_val> INT_CONST

%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp Decl ConstDecl BType ConstDef ConstInitVal BlockItem LVal ConstExp
%type <vec> BlockItemList
%type <int_val> Number
%type <unary_op_val> UnaryOp
%type <mul_op_val> MulOp
%type <add_op_val> AddOp
%type <rel_op_val> RelOp
%type <eq_op_val> EqOp

%destructor { delete $$; } <ast_val>
%destructor { delete $$; } <str_val>

%%

CompUnit
  : FuncDef {
    auto comp_unit = std::make_unique<c_ast::CompUnitAST>();
    comp_unit->func_def = unique_ptr<c_ast::BaseAST>($1);
    ast = std::move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast_node = new c_ast::FuncDefAST();
    ast_node->func_type = unique_ptr<c_ast::BaseAST>($1);
    ast_node->ident = *$2; delete $2;
    ast_node->block = unique_ptr<c_ast::BaseAST>($5);
    $$ = ast_node;
  }
  ;

FuncType
  : INT {
    auto* ast_node = new c_ast::FuncTypeAST();
    ast_node->type = c_ast::PrimitiveType::INT;     // Hard code the INT type for now.
    $$ = ast_node;
  }
  ;

Block
  : '{' BlockItemList '}' {
    auto ast_node = new c_ast::BlockAST();
    ast_node->block_items = std::move(*$2);
    delete $2;
    $$ = ast_node;
  }
  ;

BlockItemList
  : /* empty */ {
    $$ = new std::vector<<std::unique_ptr<c_ast::BastAST>>();
  }
  | BlockItemList BlockItem {
    $1->push_back(std::unique_ptr<c_ast::BaseAST>($2));
    $$ = $1;
  }
  ;

BlockItem
  : Decl { $$ = $1; }
  | Stmt { $$ = $1; }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast_node = new c_ast::StmtAST();
    
    ast_node->exp = std::unique_ptr<c_ast::BaseAST>($2);

    $$ = ast_node;
  }
  ;

Exp
  : LOrExp {
    auto ast_node = new c_ast::ExpAST();

    ast_node->lor_exp = std::unique_ptr<c_ast::BaseAST>($1);

    $$ = ast_node;
  }

PrimaryExp
  : '(' Exp ')' {
    auto ast_node = new c_ast::PrimaryASTExp();
    
    ast_node->exp = std::unique_ptr<c_ast::BaseAST>($2);

    $$ = ast_node;
  }
  | Number {
    auto ast_node = new c_ast::PrimaryASTNumber();
    
    auto num = std::make_unique<c_ast::NumberAST>();
    num->int_val = $1;
    ast_node->number = std::move(num);

    $$ = ast_node;
  }

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast_node = new c_ast::UnaryExpASTPrimary();
    
    ast_node->primary_exp = std::unique_ptr<c_ast::BaseAST>($1);

    $$ = ast_node;
  } 
  | UnaryOp UnaryExp {
    auto ast_node = new c_ast::UnaryExpASTOpUnary();

    ast_node->unary_op = $1;
    ast_node->unary_exp = std::unique_ptr<c_ast::BaseAST>($2);

    $$ = ast_node;
  }

MulExp
  : UnaryExp {
    auto ast_node = new c_ast::MulExpASTUnary();

    ast_node->unary_exp = std::unique_ptr<c_ast::BaseAST>($1);

    $$ = ast_node;
  }
  | MulExp MulOp UnaryExp {
    auto ast_node = new c_ast::MulExpASTMulUnary();

    ast_node->mul_op = $2;
    ast_node->mul_exp = std::unique_ptr<c_ast::BaseAST>($1);
    ast_node->unary_exp = std::unique_ptr<c_ast::BaseAST>($3);

    $$ = ast_node;
  }

AddExp
  : MulExp {
    auto ast_node = new c_ast::AddExpASTMul();

    ast_node->mul_exp = std::unique_ptr<c_ast::BaseAST>($1);

    $$ = ast_node;
  }
  | AddExp AddOp MulExp {
    auto ast_node = new c_ast::AddExpASTAddMul();

    ast_node->add_op = $2;
    ast_node->add_exp = std::unique_ptr<c_ast::BaseAST>($1);
    ast_node->mul_exp = std::unique_ptr<c_ast::BaseAST>($3);

    $$ = ast_node;
  }

RelExp
  : AddExp {
    auto ast_node = new c_ast::RelExpASTAdd();

    ast_node->add_exp = std::unique_ptr<c_ast::BaseAST>($1);

    $$ = ast_node;
  }
  | RelExp RelOp AddExp {
    auto ast_node = new c_ast::RelExpASTRelOpAdd();

    ast_node->rel_op = $2;
    ast_node->rel_exp = std::unique_ptr<c_ast::BaseAST>($1);
    ast_node->add_exp = std::unique_ptr<c_ast::BaseAST>($3);

    $$ = ast_node;
  }

EqExp
  : RelExp {
    auto ast_node = new c_ast::EqExpASTRel();

    ast_node->rel_exp = std::unique_ptr<c_ast::BaseAST>($1);

    $$ = ast_node;
  }
  | EqExp EqOp RelExp {
    auto ast_node = new c_ast::EqExpASTEqOpRel();

    ast_node->eq_op = $2;
    ast_node->eq_exp = std::unique_ptr<c_ast::BaseAST>($1);
    ast_node->rel_exp = std::unique_ptr<c_ast::BaseAST>($3);

    $$ = ast_node;
  }

LAndExp
  : EqExp {
    auto ast_node = new c_ast::LAndExpASTEq();

    ast_node->eq_exp = std::unique_ptr<c_ast::BaseAST>($1);

    $$ = ast_node;
  }
  | LAndExp AND EqExp {
    auto ast_node = new c_ast::LAndExpASTLAndEq();

    ast_node->land_exp = std::unique_ptr<c_ast::BaseAST>($1);
    ast_node->eq_exp = std::unique_ptr<c_ast::BaseAST>($3);

    $$ = ast_node;
  }

LOrExp
  : LAndExp {
    auto ast_node = new c_ast::LOrExpASTLAnd();

    ast_node->land_exp = std::unique_ptr<c_ast::BaseAST>($1);

    $$ = ast_node;
  }
  | LOrExp OR LAndExp {
    auto ast_node = new c_ast::LOrExpASTLOrLAnd();

    ast_node->lor_exp = std::unique_ptr<c_ast::BaseAST>($1);
    ast_node->land_exp = std::unique_ptr<c_ast::BaseAST>($3);

    $$ = ast_node;
  }

UnaryOp
  : '+' {
    $$ = c_ast::UnaryOp::PLUS;
  }
  | '-' {
    $$ = c_ast::UnaryOp::MINUS;
  }
  | '!' {
    $$ = c_ast::UnaryOp::BANG;
  }
  | '~' {
    $$ = c_ast::UnaryOp::TILDE;
  }

MulOp
  : '*' {
    $$ = c_ast::MulOp::STAR;
  }
  | '/' {
    $$ = c_ast::MulOp::SLASH;
  }
  | '%' {
    $$ = c_ast::MulOp::PERCENT;
  }

AddOp
  : '+' {
    $$ = c_ast::AddOp::PLUS;
  }
  | '-' {
    $$ = c_ast::AddOp::MINUS;
  }

RelOp
  : '<' {
    $$ = c_ast::RelOp::LT;
  }
  | LE {
    $$ = c_ast::RelOp::LE;
  }
  | '>' {
    $$ = c_ast::RelOp::GT;
  }
  | GE {
    $$ = c_ast::RelOp::GE;
  }

EqOp
  : EQ {
    $$ = c_ast::EqOp::EQ;
  }
  | NE {
    $$ = c_ast::EqOp::NE;
  }

%%

void yyerror(unique_ptr<c_ast::BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
