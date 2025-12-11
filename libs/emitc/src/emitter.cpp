#include "../include/emitter.hpp"

namespace truk::emitc {

using namespace truk::language;
using namespace truk::language::nodes;

emitter_c::emitter_c() {}

void emitter_c::emit(const base_c *root) {
  if (root) {
    root->accept(*this);
  }
}

void emitter_c::visit(const primitive_type_c &node) {}

void emitter_c::visit(const named_type_c &node) {}

void emitter_c::visit(const pointer_type_c &node) {}

void emitter_c::visit(const array_type_c &node) {}

void emitter_c::visit(const function_type_c &node) {}

void emitter_c::visit(const fn_c &node) {}

void emitter_c::visit(const struct_c &node) {}

void emitter_c::visit(const var_c &node) {}

void emitter_c::visit(const const_c &node) {}

void emitter_c::visit(const if_c &node) {}

void emitter_c::visit(const while_c &node) {}

void emitter_c::visit(const for_c &node) {}

void emitter_c::visit(const return_c &node) {}

void emitter_c::visit(const break_c &node) {}

void emitter_c::visit(const continue_c &node) {}

void emitter_c::visit(const binary_op_c &node) {}

void emitter_c::visit(const unary_op_c &node) {}

void emitter_c::visit(const call_c &node) {}

void emitter_c::visit(const index_c &node) {}

void emitter_c::visit(const member_access_c &node) {}

void emitter_c::visit(const literal_c &node) {}

void emitter_c::visit(const identifier_c &node) {}

void emitter_c::visit(const assignment_c &node) {}

void emitter_c::visit(const block_c &node) {}

void emitter_c::visit(const array_literal_c &node) {}

void emitter_c::visit(const struct_literal_c &node) {}

void emitter_c::visit(const type_param_c &node) {}

} // namespace truk::emitc
