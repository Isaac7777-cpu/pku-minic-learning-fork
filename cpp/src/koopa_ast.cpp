#include "koopa_ast.hpp"
#include "name_manager.hpp"

static constexpr const std::string_view INDENT = "\t";

namespace koopa_ast {

struct ctx {
  VarNameManager name_manager;
} current_ctx;

// Definition of `get_reprs` methods

std::string Integer::get_reprs() { return std::to_string(this->val_); }

std::string Return::get_reprs() {
  throw std::runtime_error(
      "Return statement do not have names / representations.");
}

std::string Binary::get_reprs() {
  if (!this->name) {
    this->name = current_ctx.name_manager.get_var_name();
  }
  return *this->name;
}

// Definition of Dump methods

void Type::Dump(std::ostream &out) const {

  switch (type) {
  case koopa_ast::TypeKind::I32:
    out << "i32";
    break;
  case koopa_ast::TypeKind::Unit:
    out << "void";
    break;
  }
}

void Integer::Dump(std::ostream &out) const { out << this->val_; }

void Return::Dump(std::ostream &out) const {
  // If we are returning an immediate, then return it directly
  out << INDENT << "ret ";
  this->return_val->get_reprs();
  out << "\n";
}

void Binary::Dump(std::ostream &out) const {}

void BasicBlock::Dump(std::ostream &out) const {
  if (!this->name.empty()) {
    out << this->name << ":\n";
  }

  for (auto const &v : this->insts) {
    if (v)
      v->Dump(out);
  }
}

void Function::Dump(std::ostream &out) const {
  out << "fun " << name << "(): ";
  type->Dump(out);
  out << " {\n";
  for (size_t i = 0; i < basicblocks.size(); ++i) {
    if (basicblocks[i])
      basicblocks[i]->Dump(out);

    if (i + 1 < basicblocks.size())
      out << "\n";
  }
  out << "}\n";
}

void Program::Dump(std::ostream &out) const {
  for (auto const &gv : global_values) {
    if (gv)
      gv->Dump(out);
  }
  for (auto const &f : functions) {
    if (f)
      f->Dump(out);
  }
}

} // namespace koopa_ast
