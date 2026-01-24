#pragma once

#include <string>

namespace koopa_ast {

class VarNameManager {
private:
  int var_count;

public:
  std::string get_new_var_name();
};

} // namespace koopa_ast
