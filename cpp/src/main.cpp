#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<std::string> &ast);

int main(int argc, const char *argv[]) {
  // compiler mode input_file -o output_file
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  // Open the input file, and instruct the lexer to use the file
  yyin = fopen(input, "r");
  assert(yyin);

  // Call parser function, parser will use lexer to read the file
  unique_ptr<string> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  // Output the AST (which is a string)
  cout << *ast << endl;
  return 0;
}
