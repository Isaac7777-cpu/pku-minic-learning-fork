#include "c_ast.hpp"
#include "koopa_ast.hpp"
#include <memory>

std::unique_ptr<koopa_ast::Program>
convert_to_custom_koopa_from_c_reps(std::unique_ptr<c_ast::BaseAST> ast);
