%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"

// Declare lexer function and error handling
int yylex();
void yyerror(std::unique_ptr<ast::BaseAST> &ast, const char *s);

using namespace std;

%}

%parse-param { std::unique_ptr<ast::BaseAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  ast::BaseAST *ast_val;
}

%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

%type <ast_val> FuncDef FuncType Block Stmt
%type <int_val> Number

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<ast::CompUnitAST>();
    comp_unit->func_def = unique_ptr<ast::BaseAST>($1);
    ast = std::move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast_node = new ast::FuncDefAST();
    ast_node->func_type = unique_ptr<ast::BaseAST>($1);
    ast_node->ident = *unique_ptr<string>($2);
    ast_node->block = unique_ptr<ast::BaseAST>($5);
    $$ = ast_node;
  }
  ;

FuncType
  : INT {
    $$ = new ast::FuncTypeAST();
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast_node = new ast::BlockAST();
    ast_node->stmt = unique_ptr<ast::BaseAST>($2);
    $$ = ast_node;
  }
  ;

Stmt
  : RETURN Number ';' {
    auto ast_node = new ast::StmtAST();
    
    auto num = make_unique<ast::NumberAST>();
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

void yyerror(unique_ptr<ast::BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
