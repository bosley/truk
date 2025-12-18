#pragma once

#include "keywords.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace truk::language::nodes {

class visitor_if;

enum class node_kind_e {
  PRIMITIVE_TYPE,
  NAMED_TYPE,
  POINTER_TYPE,
  ARRAY_TYPE,
  FUNCTION_TYPE,
  MAP_TYPE,
  TUPLE_TYPE,
  GENERIC_TYPE_INSTANTIATION,
  FN,
  LAMBDA,
  STRUCT,
  ENUM,
  VAR,
  CONST,
  LET,
  IF,
  WHILE,
  FOR,
  RETURN,
  BREAK,
  CONTINUE,
  DEFER,
  MATCH,
  BINARY_OP,
  UNARY_OP,
  CAST,
  CALL,
  INDEX,
  MEMBER_ACCESS,
  LITERAL,
  IDENTIFIER,
  ASSIGNMENT,
  BLOCK,
  ARRAY_LITERAL,
  STRUCT_LITERAL,
  TYPE_PARAM,
  IMPORT,
  CIMPORT,
  SHARD,
  ENUM_VALUE_ACCESS
};

enum class type_kind_e {
  PRIMITIVE,
  NAMED,
  POINTER,
  ARRAY,
  FUNCTION,
  MAP,
  TUPLE
};

class primitive_type_c;
class named_type_c;
class pointer_type_c;
class array_type_c;
class function_type_c;
class map_type_c;
class tuple_type_c;
class generic_type_instantiation_c;
class fn_c;
class lambda_c;
class struct_c;
class enum_c;
class var_c;
class const_c;
class let_c;
class if_c;
class while_c;
class for_c;
class return_c;
class break_c;
class continue_c;
class defer_c;
class match_c;
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
class shard_c;
class enum_value_access_c;

class base_c {
public:
  base_c() = delete;
  base_c(keywords_e keyword, std::size_t source_index)
      : _from_keyword(keyword), _idx(source_index) {}

  keywords_e keyword() const { return _from_keyword; }
  std::size_t source_index() const { return _idx; }

  virtual void accept(visitor_if &visitor) const = 0;
  virtual std::optional<std::string> symbol_name() const {
    return std::nullopt;
  }

  virtual node_kind_e kind() const = 0;

  virtual const primitive_type_c *as_primitive_type() const { return nullptr; }
  virtual const named_type_c *as_named_type() const { return nullptr; }
  virtual const pointer_type_c *as_pointer_type() const { return nullptr; }
  virtual const array_type_c *as_array_type() const { return nullptr; }
  virtual const function_type_c *as_function_type() const { return nullptr; }
  virtual const map_type_c *as_map_type() const { return nullptr; }
  virtual const tuple_type_c *as_tuple_type() const { return nullptr; }
  virtual const generic_type_instantiation_c *
  as_generic_type_instantiation() const {
    return nullptr;
  }
  virtual const fn_c *as_fn() const { return nullptr; }
  virtual const lambda_c *as_lambda() const { return nullptr; }
  virtual const struct_c *as_struct() const { return nullptr; }
  virtual const enum_c *as_enum() const { return nullptr; }
  virtual const var_c *as_var() const { return nullptr; }
  virtual const const_c *as_const() const { return nullptr; }
  virtual const let_c *as_let() const { return nullptr; }
  virtual const if_c *as_if() const { return nullptr; }
  virtual const while_c *as_while() const { return nullptr; }
  virtual const for_c *as_for() const { return nullptr; }
  virtual const return_c *as_return() const { return nullptr; }
  virtual const break_c *as_break() const { return nullptr; }
  virtual const continue_c *as_continue() const { return nullptr; }
  virtual const defer_c *as_defer() const { return nullptr; }
  virtual const match_c *as_match() const { return nullptr; }
  virtual const binary_op_c *as_binary_op() const { return nullptr; }
  virtual const unary_op_c *as_unary_op() const { return nullptr; }
  virtual const cast_c *as_cast() const { return nullptr; }
  virtual const call_c *as_call() const { return nullptr; }
  virtual const index_c *as_index() const { return nullptr; }
  virtual const member_access_c *as_member_access() const { return nullptr; }
  virtual const literal_c *as_literal() const { return nullptr; }
  virtual const identifier_c *as_identifier() const { return nullptr; }
  virtual const assignment_c *as_assignment() const { return nullptr; }
  virtual const block_c *as_block() const { return nullptr; }
  virtual const array_literal_c *as_array_literal() const { return nullptr; }
  virtual const struct_literal_c *as_struct_literal() const { return nullptr; }
  virtual const type_param_c *as_type_param() const { return nullptr; }
  virtual const import_c *as_import() const { return nullptr; }
  virtual const cimport_c *as_cimport() const { return nullptr; }
  virtual const shard_c *as_shard() const { return nullptr; }
  virtual const enum_value_access_c *as_enum_value_access() const {
    return nullptr;
  }

  virtual ~base_c() = default;

private:
  keywords_e _from_keyword{keywords_e::UNKNOWN_KEYWORD};
  std::size_t _idx{0};
};

using base_ptr = std::unique_ptr<base_c>;

struct identifier_s {
  std::string name;
  std::size_t source_index;

  identifier_s() = delete;
  identifier_s(std::string n, std::size_t idx)
      : name(std::move(n)), source_index(idx) {}
};

class type_c : public base_c {
public:
  type_c() = delete;
  type_c(keywords_e keyword, std::size_t source_index)
      : base_c(keyword, source_index) {}
  virtual ~type_c() = default;

  virtual type_kind_e type_kind() const = 0;

  const primitive_type_c *as_primitive_type() const override { return nullptr; }
  const named_type_c *as_named_type() const override { return nullptr; }
  const pointer_type_c *as_pointer_type() const override { return nullptr; }
  const array_type_c *as_array_type() const override { return nullptr; }
  const function_type_c *as_function_type() const override { return nullptr; }
  const map_type_c *as_map_type() const override { return nullptr; }
  const tuple_type_c *as_tuple_type() const override { return nullptr; }
};

using type_ptr = std::unique_ptr<type_c>;

struct parameter_s {
  identifier_s name;
  type_ptr type;
  bool is_variadic{false};

  parameter_s() = delete;
  parameter_s(identifier_s n, type_ptr t, bool variadic = false)
      : name(std::move(n)), type(std::move(t)), is_variadic(variadic) {}
};

struct struct_field_s {
  identifier_s name;
  type_ptr type;

  struct_field_s() = delete;
  struct_field_s(identifier_s n, type_ptr t)
      : name(std::move(n)), type(std::move(t)) {}
};

struct enum_value_s {
  identifier_s name;
  std::optional<std::int64_t> explicit_value;

  enum_value_s() = delete;
  enum_value_s(identifier_s n, std::optional<std::int64_t> val = std::nullopt)
      : name(std::move(n)), explicit_value(val) {}
};

class primitive_type_c : public type_c {
public:
  primitive_type_c() = delete;
  primitive_type_c(keywords_e primitive_keyword, std::size_t source_index)
      : type_c(primitive_keyword, source_index) {}

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::PRIMITIVE_TYPE; }
  type_kind_e type_kind() const override { return type_kind_e::PRIMITIVE; }
  const primitive_type_c *as_primitive_type() const override { return this; }
};

class named_type_c : public type_c {
public:
  named_type_c() = delete;
  named_type_c(std::size_t source_index, identifier_s name)
      : type_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _name(std::move(name)) {}

  const identifier_s &name() const { return _name; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::NAMED_TYPE; }
  type_kind_e type_kind() const override { return type_kind_e::NAMED; }
  const named_type_c *as_named_type() const override { return this; }

private:
  identifier_s _name;
};

class generic_type_instantiation_c : public type_c {
public:
  generic_type_instantiation_c() = delete;
  generic_type_instantiation_c(std::size_t source_index, identifier_s base_name,
                               std::vector<type_ptr> type_arguments)
      : type_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _base_name(std::move(base_name)),
        _type_arguments(std::move(type_arguments)) {}

  const identifier_s &base_name() const { return _base_name; }
  const std::vector<type_ptr> &type_arguments() const {
    return _type_arguments;
  }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override {
    return node_kind_e::GENERIC_TYPE_INSTANTIATION;
  }
  type_kind_e type_kind() const override { return type_kind_e::NAMED; }
  const generic_type_instantiation_c *
  as_generic_type_instantiation() const override {
    return this;
  }

private:
  identifier_s _base_name;
  std::vector<type_ptr> _type_arguments;
};

class pointer_type_c : public type_c {
public:
  pointer_type_c() = delete;
  pointer_type_c(std::size_t source_index, type_ptr pointee_type)
      : type_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _pointee_type(std::move(pointee_type)) {}

  const type_c *pointee_type() const { return _pointee_type.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::POINTER_TYPE; }
  type_kind_e type_kind() const override { return type_kind_e::POINTER; }
  const pointer_type_c *as_pointer_type() const override { return this; }

private:
  type_ptr _pointee_type;
};

class array_type_c : public type_c {
public:
  array_type_c() = delete;
  array_type_c(std::size_t source_index, type_ptr element_type,
               std::optional<std::size_t> size = std::nullopt)
      : type_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _element_type(std::move(element_type)), _size(size) {}

  const type_c *element_type() const { return _element_type.get(); }
  std::optional<std::size_t> size() const { return _size; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::ARRAY_TYPE; }
  type_kind_e type_kind() const override { return type_kind_e::ARRAY; }
  const array_type_c *as_array_type() const override { return this; }

private:
  type_ptr _element_type;
  std::optional<std::size_t> _size;
};

class function_type_c : public type_c {
public:
  function_type_c() = delete;
  function_type_c(std::size_t source_index, std::vector<type_ptr> param_types,
                  type_ptr return_type, bool has_variadic = false)
      : type_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _param_types(std::move(param_types)),
        _return_type(std::move(return_type)), _has_variadic(has_variadic) {}

  const std::vector<type_ptr> &param_types() const { return _param_types; }
  const type_c *return_type() const { return _return_type.get(); }
  bool has_variadic() const { return _has_variadic; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::FUNCTION_TYPE; }
  type_kind_e type_kind() const override { return type_kind_e::FUNCTION; }
  const function_type_c *as_function_type() const override { return this; }

private:
  std::vector<type_ptr> _param_types;
  type_ptr _return_type;
  bool _has_variadic{false};
};

class map_type_c : public type_c {
public:
  map_type_c() = delete;
  map_type_c(std::size_t source_index, type_ptr key_type, type_ptr value_type)
      : type_c(keywords_e::MAP, source_index), _key_type(std::move(key_type)),
        _value_type(std::move(value_type)) {}

  const type_c *key_type() const { return _key_type.get(); }
  const type_c *value_type() const { return _value_type.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::MAP_TYPE; }
  type_kind_e type_kind() const override { return type_kind_e::MAP; }
  const map_type_c *as_map_type() const override { return this; }

private:
  type_ptr _key_type;
  type_ptr _value_type;
};

class tuple_type_c : public type_c {
public:
  tuple_type_c() = delete;
  tuple_type_c(std::size_t source_index, std::vector<type_ptr> element_types)
      : type_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _element_types(std::move(element_types)) {}

  const std::vector<type_ptr> &element_types() const { return _element_types; }
  std::size_t arity() const { return _element_types.size(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::TUPLE_TYPE; }
  type_kind_e type_kind() const override { return type_kind_e::TUPLE; }
  const tuple_type_c *as_tuple_type() const override { return this; }

private:
  std::vector<type_ptr> _element_types;
};

class fn_c : public base_c {
public:
  fn_c() = delete;
  fn_c(std::size_t source_index, identifier_s name,
       std::vector<parameter_s> params, type_ptr return_type,
       std::optional<base_ptr> body, bool is_extern = false)
      : base_c(keywords_e::FN, source_index), _name(std::move(name)),
        _params(std::move(params)), _return_type(std::move(return_type)),
        _body(std::move(body)), _is_extern(is_extern) {}

  const identifier_s &name() const { return _name; }
  const std::vector<parameter_s> &params() const { return _params; }
  const type_c *return_type() const { return _return_type.get(); }
  const base_c *body() const { return _body ? _body->get() : nullptr; }
  bool is_extern() const { return _is_extern; }

  void accept(visitor_if &visitor) const override;
  std::optional<std::string> symbol_name() const override { return _name.name; }
  node_kind_e kind() const override { return node_kind_e::FN; }
  const fn_c *as_fn() const override { return this; }

private:
  identifier_s _name;
  std::vector<parameter_s> _params;
  type_ptr _return_type;
  std::optional<base_ptr> _body;
  bool _is_extern{false};
};

class lambda_c : public base_c {
public:
  lambda_c() = delete;
  lambda_c(std::size_t source_index, std::vector<parameter_s> params,
           type_ptr return_type, base_ptr body, bool is_capturing = false)
      : base_c(keywords_e::FN, source_index), _params(std::move(params)),
        _return_type(std::move(return_type)), _body(std::move(body)),
        _is_capturing(is_capturing) {}

  const std::vector<parameter_s> &params() const { return _params; }
  const type_c *return_type() const { return _return_type.get(); }
  const base_c *body() const { return _body.get(); }
  bool is_capturing() const { return _is_capturing; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::LAMBDA; }
  const lambda_c *as_lambda() const override { return this; }

private:
  std::vector<parameter_s> _params;
  type_ptr _return_type;
  base_ptr _body;
  bool _is_capturing{false};
};

class struct_c : public base_c {
public:
  struct_c() = delete;
  struct_c(std::size_t source_index, identifier_s name,
           std::vector<identifier_s> type_params,
           std::vector<struct_field_s> fields, bool is_extern = false)
      : base_c(keywords_e::STRUCT, source_index), _name(std::move(name)),
        _type_params(std::move(type_params)), _fields(std::move(fields)),
        _is_extern(is_extern) {}

  const identifier_s &name() const { return _name; }
  const std::vector<identifier_s> &type_params() const { return _type_params; }
  const std::vector<struct_field_s> &fields() const { return _fields; }
  bool is_extern() const { return _is_extern; }
  bool is_generic() const { return !_type_params.empty(); }

  void accept(visitor_if &visitor) const override;
  std::optional<std::string> symbol_name() const override { return _name.name; }
  node_kind_e kind() const override { return node_kind_e::STRUCT; }
  const struct_c *as_struct() const override { return this; }

private:
  identifier_s _name;
  std::vector<identifier_s> _type_params;
  std::vector<struct_field_s> _fields;
  bool _is_extern{false};
};

class enum_c : public base_c {
public:
  enum_c() = delete;
  enum_c(std::size_t source_index, identifier_s name, type_ptr backing_type,
         std::vector<enum_value_s> values, bool is_extern = false)
      : base_c(keywords_e::ENUM, source_index), _name(std::move(name)),
        _backing_type(std::move(backing_type)), _values(std::move(values)),
        _is_extern(is_extern) {}

  const identifier_s &name() const { return _name; }
  const type_c *backing_type() const { return _backing_type.get(); }
  const std::vector<enum_value_s> &values() const { return _values; }
  bool is_extern() const { return _is_extern; }

  void accept(visitor_if &visitor) const override;
  std::optional<std::string> symbol_name() const override { return _name.name; }
  node_kind_e kind() const override { return node_kind_e::ENUM; }
  const enum_c *as_enum() const override { return this; }

private:
  identifier_s _name;
  type_ptr _backing_type;
  std::vector<enum_value_s> _values;
  bool _is_extern{false};
};

class var_c : public base_c {
public:
  var_c() = delete;
  var_c(std::size_t source_index, identifier_s name, type_ptr type,
        std::optional<base_ptr> initializer = std::nullopt,
        bool is_extern = false)
      : base_c(keywords_e::VAR, source_index), _name(std::move(name)),
        _type(std::move(type)), _initializer(std::move(initializer)),
        _is_extern(is_extern) {}

  const identifier_s &name() const { return _name; }
  const type_c *type() const { return _type.get(); }
  const base_c *initializer() const {
    return _initializer ? _initializer->get() : nullptr;
  }
  bool is_extern() const { return _is_extern; }

  void accept(visitor_if &visitor) const override;
  std::optional<std::string> symbol_name() const override { return _name.name; }
  node_kind_e kind() const override { return node_kind_e::VAR; }
  const var_c *as_var() const override { return this; }

private:
  identifier_s _name;
  type_ptr _type;
  std::optional<base_ptr> _initializer;
  bool _is_extern{false};
};

class const_c : public base_c {
public:
  const_c() = delete;
  const_c(std::size_t source_index, identifier_s name, type_ptr type,
          base_ptr value)
      : base_c(keywords_e::CONST, source_index), _name(std::move(name)),
        _type(std::move(type)), _value(std::move(value)) {}

  const identifier_s &name() const { return _name; }
  const type_c *type() const { return _type.get(); }
  const base_c *value() const { return _value.get(); }

  void accept(visitor_if &visitor) const override;
  std::optional<std::string> symbol_name() const override { return _name.name; }
  node_kind_e kind() const override { return node_kind_e::CONST; }
  const const_c *as_const() const override { return this; }

private:
  identifier_s _name;
  type_ptr _type;
  base_ptr _value;
};

class let_c : public base_c {
public:
  let_c() = delete;
  let_c(std::size_t source_index, std::vector<identifier_s> names,
        base_ptr initializer)
      : base_c(keywords_e::LET, source_index), _names(std::move(names)),
        _initializer(std::move(initializer)) {}

  const std::vector<identifier_s> &names() const { return _names; }
  const base_c *initializer() const { return _initializer.get(); }
  const std::vector<type_ptr> &inferred_types() const {
    return _inferred_types;
  }

  bool is_single() const { return _names.size() == 1; }
  bool is_destructuring() const { return _names.size() > 1; }

  void set_inferred_types(std::vector<type_ptr> types) const {
    _inferred_types = std::move(types);
  }

  void accept(visitor_if &visitor) const override;
  std::optional<std::string> symbol_name() const override {
    return is_single() ? std::optional<std::string>(_names[0].name)
                       : std::nullopt;
  }
  node_kind_e kind() const override { return node_kind_e::LET; }
  const let_c *as_let() const override { return this; }

private:
  std::vector<identifier_s> _names;
  base_ptr _initializer;
  mutable std::vector<type_ptr> _inferred_types;
};

class if_c : public base_c {
public:
  if_c() = delete;
  if_c(std::size_t source_index, base_ptr condition, base_ptr then_block,
       std::optional<base_ptr> else_block = std::nullopt)
      : base_c(keywords_e::IF, source_index), _condition(std::move(condition)),
        _then_block(std::move(then_block)), _else_block(std::move(else_block)) {
  }

  const base_c *condition() const { return _condition.get(); }
  const base_c *then_block() const { return _then_block.get(); }
  const base_c *else_block() const {
    return _else_block ? _else_block->get() : nullptr;
  }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::IF; }
  const if_c *as_if() const override { return this; }

private:
  base_ptr _condition;
  base_ptr _then_block;
  std::optional<base_ptr> _else_block;
};

class while_c : public base_c {
public:
  while_c() = delete;
  while_c(std::size_t source_index, base_ptr condition, base_ptr body)
      : base_c(keywords_e::WHILE, source_index),
        _condition(std::move(condition)), _body(std::move(body)) {}

  const base_c *condition() const { return _condition.get(); }
  const base_c *body() const { return _body.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::WHILE; }
  const while_c *as_while() const override { return this; }

private:
  base_ptr _condition;
  base_ptr _body;
};

class for_c : public base_c {
public:
  for_c() = delete;
  for_c(std::size_t source_index, std::optional<base_ptr> init,
        std::optional<base_ptr> condition, std::optional<base_ptr> post,
        base_ptr body)
      : base_c(keywords_e::FOR, source_index), _init(std::move(init)),
        _condition(std::move(condition)), _post(std::move(post)),
        _body(std::move(body)) {}

  const base_c *init() const { return _init ? _init->get() : nullptr; }
  const base_c *condition() const {
    return _condition ? _condition->get() : nullptr;
  }
  const base_c *post() const { return _post ? _post->get() : nullptr; }
  const base_c *body() const { return _body.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::FOR; }
  const for_c *as_for() const override { return this; }

private:
  std::optional<base_ptr> _init;
  std::optional<base_ptr> _condition;
  std::optional<base_ptr> _post;
  base_ptr _body;
};

class return_c : public base_c {
public:
  return_c() = delete;
  return_c(std::size_t source_index, std::vector<base_ptr> expressions)
      : base_c(keywords_e::RETURN, source_index),
        _expressions(std::move(expressions)) {}

  const std::vector<base_ptr> &expressions() const { return _expressions; }
  bool is_void() const { return _expressions.empty(); }
  bool is_single() const { return _expressions.size() == 1; }
  bool is_multiple() const { return _expressions.size() > 1; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::RETURN; }
  const return_c *as_return() const override { return this; }

private:
  std::vector<base_ptr> _expressions;
};

class break_c : public base_c {
public:
  break_c() = delete;
  break_c(std::size_t source_index) : base_c(keywords_e::BREAK, source_index) {}

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::BREAK; }
  const break_c *as_break() const override { return this; }
};

class continue_c : public base_c {
public:
  continue_c() = delete;
  continue_c(std::size_t source_index)
      : base_c(keywords_e::CONTINUE, source_index) {}

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::CONTINUE; }
  const continue_c *as_continue() const override { return this; }
};

class defer_c : public base_c {
public:
  defer_c() = delete;
  defer_c(std::size_t source_index, base_ptr deferred_code)
      : base_c(keywords_e::DEFER, source_index),
        _deferred_code(std::move(deferred_code)) {}

  const base_c *deferred_code() const { return _deferred_code.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::DEFER; }
  const defer_c *as_defer() const override { return this; }

private:
  base_ptr _deferred_code;
};

enum class binary_op_e {
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
  EQ,
  NE,
  LT,
  LE,
  GT,
  GE,
  AND,
  OR,
  BITWISE_AND,
  BITWISE_OR,
  BITWISE_XOR,
  LEFT_SHIFT,
  RIGHT_SHIFT
};

class binary_op_c : public base_c {
public:
  binary_op_c() = delete;
  binary_op_c(std::size_t source_index, binary_op_e op, base_ptr left,
              base_ptr right)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index), _op(op),
        _left(std::move(left)), _right(std::move(right)) {}

  binary_op_e op() const { return _op; }
  const base_c *left() const { return _left.get(); }
  const base_c *right() const { return _right.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::BINARY_OP; }
  const binary_op_c *as_binary_op() const override { return this; }

private:
  binary_op_e _op;
  base_ptr _left;
  base_ptr _right;
};

enum class unary_op_e { NEG, NOT, BITWISE_NOT, ADDRESS_OF, DEREF };

class unary_op_c : public base_c {
public:
  unary_op_c() = delete;
  unary_op_c(std::size_t source_index, unary_op_e op, base_ptr operand)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index), _op(op),
        _operand(std::move(operand)) {}

  unary_op_e op() const { return _op; }
  const base_c *operand() const { return _operand.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::UNARY_OP; }
  const unary_op_c *as_unary_op() const override { return this; }

private:
  unary_op_e _op;
  base_ptr _operand;
};

class cast_c : public base_c {
public:
  cast_c() = delete;
  cast_c(std::size_t source_index, base_ptr expression, type_ptr target_type)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _expression(std::move(expression)),
        _target_type(std::move(target_type)) {}

  const base_c *expression() const { return _expression.get(); }
  const type_c *target_type() const { return _target_type.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::CAST; }
  const cast_c *as_cast() const override { return this; }

private:
  base_ptr _expression;
  type_ptr _target_type;
};

class call_c : public base_c {
public:
  call_c() = delete;
  call_c(std::size_t source_index, base_ptr callee,
         std::vector<base_ptr> arguments)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _callee(std::move(callee)), _arguments(std::move(arguments)) {}

  const base_c *callee() const { return _callee.get(); }
  const std::vector<base_ptr> &arguments() const { return _arguments; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::CALL; }
  const call_c *as_call() const override { return this; }

private:
  base_ptr _callee;
  std::vector<base_ptr> _arguments;
};

class index_c : public base_c {
public:
  index_c() = delete;
  index_c(std::size_t source_index, base_ptr object, base_ptr index)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _object(std::move(object)), _index(std::move(index)) {}

  const base_c *object() const { return _object.get(); }
  const base_c *index() const { return _index.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::INDEX; }
  const index_c *as_index() const override { return this; }

private:
  base_ptr _object;
  base_ptr _index;
};

class member_access_c : public base_c {
public:
  member_access_c() = delete;
  member_access_c(std::size_t source_index, base_ptr object, identifier_s field)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _object(std::move(object)), _field(std::move(field)) {}

  const base_c *object() const { return _object.get(); }
  const identifier_s &field() const { return _field; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::MEMBER_ACCESS; }
  const member_access_c *as_member_access() const override { return this; }

private:
  base_ptr _object;
  identifier_s _field;
};

class type_param_c : public base_c {
public:
  type_param_c() = delete;
  type_param_c(std::size_t source_index, type_ptr type)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _type(std::move(type)) {}

  const type_c *type() const { return _type.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::TYPE_PARAM; }
  const type_param_c *as_type_param() const override { return this; }

private:
  type_ptr _type;
};

enum class literal_type_e { INTEGER, FLOAT, STRING, CHAR, BOOL, NIL };

class literal_c : public base_c {
public:
  literal_c() = delete;
  literal_c(std::size_t source_index, literal_type_e type, std::string value)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index), _type(type),
        _value(std::move(value)) {}

  literal_type_e type() const { return _type; }
  const std::string &value() const { return _value; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::LITERAL; }
  const literal_c *as_literal() const override { return this; }

private:
  literal_type_e _type;
  std::string _value;
};

class identifier_c : public base_c {
public:
  identifier_c() = delete;
  identifier_c(std::size_t source_index, identifier_s id)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index), _id(std::move(id)) {}

  const identifier_s &id() const { return _id; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::IDENTIFIER; }
  const identifier_c *as_identifier() const override { return this; }

private:
  identifier_s _id;
};

class assignment_c : public base_c {
public:
  assignment_c() = delete;
  assignment_c(std::size_t source_index, base_ptr target, base_ptr value)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _target(std::move(target)), _value(std::move(value)) {}

  const base_c *target() const { return _target.get(); }
  const base_c *value() const { return _value.get(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::ASSIGNMENT; }
  const assignment_c *as_assignment() const override { return this; }

private:
  base_ptr _target;
  base_ptr _value;
};

class block_c : public base_c {
public:
  block_c() = delete;
  block_c(std::size_t source_index, std::vector<base_ptr> statements)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _statements(std::move(statements)) {}

  const std::vector<base_ptr> &statements() const { return _statements; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::BLOCK; }
  const block_c *as_block() const override { return this; }

private:
  std::vector<base_ptr> _statements;
};

class array_literal_c : public base_c {
public:
  array_literal_c() = delete;
  array_literal_c(std::size_t source_index, std::vector<base_ptr> elements)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _elements(std::move(elements)) {}

  const std::vector<base_ptr> &elements() const { return _elements; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::ARRAY_LITERAL; }
  const array_literal_c *as_array_literal() const override { return this; }

private:
  std::vector<base_ptr> _elements;
};

struct field_initializer_s {
  identifier_s field_name;
  base_ptr value;

  field_initializer_s() = delete;
  field_initializer_s(identifier_s name, base_ptr val)
      : field_name(std::move(name)), value(std::move(val)) {}
};

class struct_literal_c : public base_c {
public:
  struct_literal_c() = delete;
  struct_literal_c(std::size_t source_index, identifier_s struct_name,
                   std::vector<type_ptr> type_arguments,
                   std::vector<field_initializer_s> field_initializers)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _struct_name(std::move(struct_name)),
        _type_arguments(std::move(type_arguments)),
        _field_initializers(std::move(field_initializers)) {}

  const identifier_s &struct_name() const { return _struct_name; }
  const std::vector<type_ptr> &type_arguments() const {
    return _type_arguments;
  }
  const std::vector<field_initializer_s> &field_initializers() const {
    return _field_initializers;
  }
  bool is_generic() const { return !_type_arguments.empty(); }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::STRUCT_LITERAL; }
  const struct_literal_c *as_struct_literal() const override { return this; }

private:
  identifier_s _struct_name;
  std::vector<type_ptr> _type_arguments;
  std::vector<field_initializer_s> _field_initializers;
};

class import_c : public base_c {
public:
  import_c() = delete;
  import_c(std::size_t source_index, std::string path)
      : base_c(keywords_e::IMPORT, source_index), _path(std::move(path)) {}

  const std::string &path() const { return _path; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::IMPORT; }
  const import_c *as_import() const override { return this; }

private:
  std::string _path;
};

struct c_import_s {
  std::string path;
  bool is_angle_bracket;
};

class cimport_c : public base_c {
public:
  cimport_c() = delete;
  cimport_c(std::size_t source_index, std::string path, bool is_angle_bracket)
      : base_c(keywords_e::CIMPORT, source_index), _path(std::move(path)),
        _is_angle_bracket(is_angle_bracket) {}

  const std::string &path() const { return _path; }
  bool is_angle_bracket() const { return _is_angle_bracket; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::CIMPORT; }
  const cimport_c *as_cimport() const override { return this; }

private:
  std::string _path;
  bool _is_angle_bracket;
};

class shard_c : public base_c {
public:
  shard_c() = delete;
  shard_c(std::size_t source_index, std::string name)
      : base_c(keywords_e::SHARD, source_index), _name(std::move(name)) {}

  const std::string &name() const { return _name; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::SHARD; }
  const shard_c *as_shard() const override { return this; }

private:
  std::string _name;
};

class enum_value_access_c : public base_c {
public:
  enum_value_access_c() = delete;
  enum_value_access_c(std::size_t source_index, identifier_s enum_name,
                      identifier_s value_name)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _enum_name(std::move(enum_name)), _value_name(std::move(value_name)) {}

  const identifier_s &enum_name() const { return _enum_name; }
  const identifier_s &value_name() const { return _value_name; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::ENUM_VALUE_ACCESS; }
  const enum_value_access_c *as_enum_value_access() const override {
    return this;
  }

private:
  identifier_s _enum_name;
  identifier_s _value_name;
};

struct match_case_s {
  base_ptr pattern;
  base_ptr body;
  bool is_wildcard;

  match_case_s() = delete;
  match_case_s(base_ptr pat, base_ptr b, bool wildcard = false)
      : pattern(std::move(pat)), body(std::move(b)), is_wildcard(wildcard) {}
};

class match_c : public base_c {
public:
  match_c() = delete;
  match_c(std::size_t source_index, base_ptr scrutinee,
          std::vector<match_case_s> cases)
      : base_c(keywords_e::MATCH, source_index),
        _scrutinee(std::move(scrutinee)), _cases(std::move(cases)) {}

  const base_c *scrutinee() const { return _scrutinee.get(); }
  const std::vector<match_case_s> &cases() const { return _cases; }

  void accept(visitor_if &visitor) const override;
  node_kind_e kind() const override { return node_kind_e::MATCH; }
  const match_c *as_match() const override { return this; }

private:
  base_ptr _scrutinee;
  std::vector<match_case_s> _cases;
};

} // namespace truk::language::nodes