#include "c_ast.hpp"
#include "codegen.hpp"
#include "ir_builder.hpp"
#include "koopa.h"
#include "koopa_ast.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

extern FILE *yyin;
extern int yyparse(std::unique_ptr<c_ast::BaseAST> &ast);

enum class COMPILE_MODE { KOOPA_IR, RISC_V };

COMPILE_MODE parse_compile_mode(std::string arg) {
  std::string mode = arg.substr(1);
  if (mode == "koopa") {
    return COMPILE_MODE::KOOPA_IR;
  }
  if (mode == "riscv") {
    return COMPILE_MODE::RISC_V;
  }

  std::cerr << "error: unknown compile mode '-" << mode
            << "'. Expected '-koopa' or '-riscv'.\n";
  std::exit(1);
}

int main(int argc, const char *argv[]) {
  // Compiler mode input_file -o output_file
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  COMPILE_MODE compile_mode = parse_compile_mode(mode);

  // Open the input file, and instruct the lexer to use the file
  yyin = fopen(input, "r");
  assert(yyin);

  std::ofstream output_stream(output);
  if (output_stream.is_open() == false) {
    std::cerr << "Unable to write to output file: " << output << std::endl;
    return 1;
  }

  // Call parser function, parser will use lexer to read the file
  std::unique_ptr<c_ast::BaseAST> c_ast;
  auto c_parse_ret = yyparse(c_ast);
  assert(!c_parse_ret);

  // Output the AST (which is a string)
  std::cout << "The C_AST: " << std::endl;
  c_ast->Dump();
  std::cout << std::endl << std::endl;

  // Translate to Koopa IR
  auto ret_in_koopa = convert_to_custom_koopa_from_c_reps(std::move(c_ast));
  // Create a string stream to store the results
  std::stringstream koopa_ir_ss;
  ret_in_koopa->Dump(koopa_ir_ss);
  // Output to the file
  if (compile_mode == COMPILE_MODE::KOOPA_IR) {
    output_stream << koopa_ir_ss.str();
    return 0;
  }

  // Convert to koopa program representation
  koopa_program_t koopa_program;
  koopa_error_code_t koopa_parse_ret =
      koopa_parse_from_string(koopa_ir_ss.str().c_str(), &koopa_program);
  assert(koopa_parse_ret == KOOPA_EC_SUCCESS);

  // Translate to koopa raw program
  koopa_raw_program_builder_t koopa_raw_builder =
      koopa_new_raw_program_builder();
  koopa_raw_program_t koopa_raw_program =
      koopa_build_raw_program(koopa_raw_builder, koopa_program);
  koopa_delete_program(koopa_program);

  // Generate RISC_V

  if (compile_mode == COMPILE_MODE::RISC_V) {
    CodeGenUnit gen(output_stream);
    gen.generate(koopa_raw_program);
  }

  koopa_delete_raw_program_builder(koopa_raw_builder);

  return 0;
}
