#pragma once

namespace truk::language::nodes {

class primitive_type_c;
class named_type_c;
class pointer_type_c;
class array_type_c;
class function_type_c;
class fn_c;
class struct_c;
class var_c;
class const_c;
class if_c;
class while_c;
class for_c;
class return_c;
class break_c;
class continue_c;
class defer_c;
class binary_op_c;
class unary_op_c;
class cast_c;
class call_c;
class index_c;
class member_access_c;
class literal_c;
class identifier_c;
class assignment_c;
class block_c;
class array_literal_c;
class struct_literal_c;
class type_param_c;
class import_c;
class cimport_c;

class visitor_if {
public:
  virtual ~visitor_if() = default;

  virtual void visit(const primitive_type_c &node) = 0;
  virtual void visit(const named_type_c &node) = 0;
  virtual void visit(const pointer_type_c &node) = 0;
  virtual void visit(const array_type_c &node) = 0;
  virtual void visit(const function_type_c &node) = 0;
  virtual void visit(const fn_c &node) = 0;
  virtual void visit(const struct_c &node) = 0;
  virtual void visit(const var_c &node) = 0;
  virtual void visit(const const_c &node) = 0;
  virtual void visit(const if_c &node) = 0;
  virtual void visit(const while_c &node) = 0;
  virtual void visit(const for_c &node) = 0;
  virtual void visit(const return_c &node) = 0;
  virtual void visit(const break_c &node) = 0;
  virtual void visit(const continue_c &node) = 0;
  virtual void visit(const defer_c &node) = 0;
  virtual void visit(const binary_op_c &node) = 0;
  virtual void visit(const unary_op_c &node) = 0;
  virtual void visit(const cast_c &node) = 0;
  virtual void visit(const call_c &node) = 0;
  virtual void visit(const index_c &node) = 0;
  virtual void visit(const member_access_c &node) = 0;
  virtual void visit(const literal_c &node) = 0;
  virtual void visit(const identifier_c &node) = 0;
  virtual void visit(const assignment_c &node) = 0;
  virtual void visit(const block_c &node) = 0;
  virtual void visit(const array_literal_c &node) = 0;
  virtual void visit(const struct_literal_c &node) = 0;
  virtual void visit(const type_param_c &node) = 0;
  virtual void visit(const import_c &node) = 0;
  virtual void visit(const cimport_c &node) = 0;
};

} // namespace truk::language::nodes
