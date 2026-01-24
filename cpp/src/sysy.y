%code requires {
  #include <memory>
  #include <string>
  #include "c_ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "c_ast.hpp"

// Declare lexer function and error handling
int yylex();
void yyerror(std::unique_ptr<c_ast::BaseAST> &ast, const char *s);

using namespace std;

%}

%parse-param { std::unique_ptr<c_ast::BaseAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  c_ast::UnaryOp op_val;
  c_ast::BaseAST *ast_val;
}

%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp
%type <int_val> Number
%type <op_val> UnaryOp

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
    $$ = new c_ast::FuncTypeAST();
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast_node = new c_ast::BlockAST();
    ast_node->stmt = unique_ptr<c_ast::BaseAST>($2);
    $$ = ast_node;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast_node = new c_ast::StmtAST();
    
    ast_node->exp = std::unique_ptr<c_ast::BaseAST>($2);

    $$ = ast_node;
  }
  ;

Exp
  : UnaryExp {
    auto ast_node = new c_ast::ExpAST();

    ast_node->unary_exp = std::unique_ptr<c_ast::BaseAST>($1);

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

%%

void yyerror(unique_ptr<c_ast::BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
