#include <language/node.hpp>
#include <language/visitor.hpp>

namespace truk::language::nodes {

void primitive_type_c::accept(visitor_if &visitor) const {
  visitor.visit(*this);
}

void named_type_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void pointer_type_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void array_type_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void function_type_c::accept(visitor_if &visitor) const {
  visitor.visit(*this);
}

void map_type_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void tuple_type_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void fn_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void lambda_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void struct_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void enum_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void var_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void const_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void let_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void if_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void while_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void for_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void return_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void break_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void continue_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void defer_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void binary_op_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void unary_op_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void cast_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void call_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void index_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void member_access_c::accept(visitor_if &visitor) const {
  visitor.visit(*this);
}

void literal_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void identifier_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void assignment_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void block_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void array_literal_c::accept(visitor_if &visitor) const {
  visitor.visit(*this);
}

void struct_literal_c::accept(visitor_if &visitor) const {
  visitor.visit(*this);
}

void type_param_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void import_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void cimport_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void shard_c::accept(visitor_if &visitor) const { visitor.visit(*this); }

void enum_value_access_c::accept(visitor_if &visitor) const {
  visitor.visit(*this);
}

} // namespace truk::language::nodes
