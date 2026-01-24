#include "name_manager.hpp"
#include <string>

std::string koopa_ast::VarNameManager::get_var_name() {
  return "%" + std::to_string(this->var_count++);
}
