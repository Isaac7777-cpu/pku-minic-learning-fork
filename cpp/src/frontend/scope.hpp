#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>

namespace koopa_ast {
class Value;
}

struct ConstantSymbol {
  int32_t value;
};

struct VariableSymbol {
  koopa_ast::Value *address; // TODO: implement the variable in 4.2
};

using Symbol = std::variant<ConstantSymbol, VariableSymbol>;

class Scope {
public:
  explicit Scope(const Scope *parent = nullptr);
  bool ConstainsLocal(const std::string &name) const;
  const Symbol &Lookup(const std::string &name) const;
  int32_t LookupConstant(const std::string &name) const;
  void DefineConstant(const std::string &name, int32_t value);
  void DefineVariable(const std::string &name, koopa_ast::Value *address);

private:
  void Define(const std::string &name, Symbol symbol);
  const Scope *parent_;
  std::unordered_map<std::string, Symbol> symbols_;
};
