#include "scope.hpp"

#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

Scope::Scope(const Scope *parent) : parent_(parent) {}

bool Scope::ConstainsLocal(const std::string &name) const {
  return symbols_.find(name) != symbols_.end();
}

const Symbol &Scope::Lookup(const std::string &name) const {
  for (auto *scope = this; scope; scope = scope->parent_) {
    auto it = scope->symbols_.find(name);

    if (it != scope->symbols_.end())
      return it->second;
  }

  throw std::runtime_error("Undefined symbol: " + name);
}

int32_t Scope::LookupConstant(const std::string &name) const {
  const Symbol &symbol = Lookup(name);

  if (auto *constant = std::get_if<ConstantSymbol>(&symbol)) {
    return constant->value;
  }

  throw std::runtime_error("Variable used in constant expression: " + name);
}

void Scope::DefineConstant(const std::string &name, int32_t value) {
  Define(name, ConstantSymbol{value});
}

void Scope::DefineVariable(const std::string &name, koopa_ast::Value *value) {
  Define(name, VariableSymbol{value});
}

void Scope::Define(const std::string &name, Symbol symbol) {
  if (!symbols_.emplace(name, std::move(symbol)).second) {
    throw std::runtime_error("Duplicate symbol: " + name);
  }
}
