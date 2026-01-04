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
  c_ast::BaseAST *ast_val;
}

%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

%type <ast_val> FuncDef FuncType Block Stmt
%type <int_val> Number

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<c_ast::CompUnitAST>();
    comp_unit->func_def = unique_ptr<c_ast::BaseAST>($1);
    ast = std::move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast_node = new c_ast::FuncDefAST();
    ast_node->func_type = unique_ptr<c_ast::BaseAST>($1);
    ast_node->ident = *unique_ptr<string>($2);
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
  : RETURN Number ';' {
    auto ast_node = new c_ast::StmtAST();
    
    auto num = make_unique<c_ast::NumberAST>();
    num->int_val = $2;

    ast_node->number = std::move(num);

    $$ = ast_node;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

%%

void yyerror(unique_ptr<c_ast::BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
