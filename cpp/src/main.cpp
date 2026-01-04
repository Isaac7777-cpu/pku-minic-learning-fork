#include "c_ast.hpp"
#include "koopa_ast.hpp"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <memory>

// using namespace std;

extern FILE *yyin;
extern int yyparse(std::unique_ptr<c_ast::BaseAST> &ast);

int main(int argc, const char *argv[]) {
  // Compiler mode input_file -o output_file
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  // Open the input file, and instruct the lexer to use the file
  yyin = fopen(input, "r");
  assert(yyin);

  // Call parser function, parser will use lexer to read the file
  std::unique_ptr<c_ast::BaseAST> c_ast;
  auto ret = yyparse(c_ast);
  assert(!ret);

  // Output the AST (which is a string)
  std::cout << "The C_AST: " << std::endl;
  c_ast->Dump();
  std::cout << std::endl << std::endl;

  // Translate to Koopa IR
  auto ret_in_koopa = convert_to_custom_koopa_from_c_reps(std::move(c_ast));

  std::cout << "The Koopa IR: " << std::endl << std::endl;
  std::cout << "```" << std::endl;
  ret_in_koopa->Dump(std::cout);
  std::cout << "```" << std::endl;

  return 0;
}
