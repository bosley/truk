#pragma once

#include "keywords.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace truk::language::nodes {

class visitor_if;

class base_c {
public:
  base_c() = delete;
  base_c(keywords_e keyword, std::size_t source_index)
      : _from_keyword(keyword), _idx(source_index) {}

  keywords_e keyword() const { return _from_keyword; }
  std::size_t source_index() const { return _idx; }

  virtual void accept(visitor_if &visitor) const = 0;

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

class primitive_type_c : public type_c {
public:
  primitive_type_c() = delete;
  primitive_type_c(keywords_e primitive_keyword, std::size_t source_index)
      : type_c(primitive_keyword, source_index) {}

  void accept(visitor_if &visitor) const override;
};

class named_type_c : public type_c {
public:
  named_type_c() = delete;
  named_type_c(std::size_t source_index, identifier_s name)
      : type_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _name(std::move(name)) {}

  const identifier_s &name() const { return _name; }

  void accept(visitor_if &visitor) const override;

private:
  identifier_s _name;
};

class pointer_type_c : public type_c {
public:
  pointer_type_c() = delete;
  pointer_type_c(std::size_t source_index, type_ptr pointee_type)
      : type_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _pointee_type(std::move(pointee_type)) {}

  const type_c *pointee_type() const { return _pointee_type.get(); }

  void accept(visitor_if &visitor) const override;

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

private:
  std::vector<type_ptr> _param_types;
  type_ptr _return_type;
  bool _has_variadic{false};
};

class fn_c : public base_c {
public:
  fn_c() = delete;
  fn_c(std::size_t source_index, identifier_s name,
       std::vector<parameter_s> params, type_ptr return_type, base_ptr body)
      : base_c(keywords_e::FN, source_index), _name(std::move(name)),
        _params(std::move(params)), _return_type(std::move(return_type)),
        _body(std::move(body)) {}

  const identifier_s &name() const { return _name; }
  const std::vector<parameter_s> &params() const { return _params; }
  const type_c *return_type() const { return _return_type.get(); }
  const base_c *body() const { return _body.get(); }

  void accept(visitor_if &visitor) const override;

private:
  identifier_s _name;
  std::vector<parameter_s> _params;
  type_ptr _return_type;
  base_ptr _body;
};

class struct_c : public base_c {
public:
  struct_c() = delete;
  struct_c(std::size_t source_index, identifier_s name,
           std::vector<struct_field_s> fields)
      : base_c(keywords_e::STRUCT, source_index), _name(std::move(name)),
        _fields(std::move(fields)) {}

  const identifier_s &name() const { return _name; }
  const std::vector<struct_field_s> &fields() const { return _fields; }

  void accept(visitor_if &visitor) const override;

private:
  identifier_s _name;
  std::vector<struct_field_s> _fields;
};

class var_c : public base_c {
public:
  var_c() = delete;
  var_c(std::size_t source_index, identifier_s name, type_ptr type,
        std::optional<base_ptr> initializer = std::nullopt)
      : base_c(keywords_e::VAR, source_index), _name(std::move(name)),
        _type(std::move(type)), _initializer(std::move(initializer)) {}

  const identifier_s &name() const { return _name; }
  const type_c *type() const { return _type.get(); }
  const base_c *initializer() const {
    return _initializer ? _initializer->get() : nullptr;
  }

  void accept(visitor_if &visitor) const override;

private:
  identifier_s _name;
  type_ptr _type;
  std::optional<base_ptr> _initializer;
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

private:
  identifier_s _name;
  type_ptr _type;
  base_ptr _value;
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

private:
  std::optional<base_ptr> _init;
  std::optional<base_ptr> _condition;
  std::optional<base_ptr> _post;
  base_ptr _body;
};

class return_c : public base_c {
public:
  return_c() = delete;
  return_c(std::size_t source_index,
           std::optional<base_ptr> expression = std::nullopt)
      : base_c(keywords_e::RETURN, source_index),
        _expression(std::move(expression)) {}

  const base_c *expression() const {
    return _expression ? _expression->get() : nullptr;
  }

  void accept(visitor_if &visitor) const override;

private:
  std::optional<base_ptr> _expression;
};

class break_c : public base_c {
public:
  break_c() = delete;
  break_c(std::size_t source_index) : base_c(keywords_e::BREAK, source_index) {}

  void accept(visitor_if &visitor) const override;
};

class continue_c : public base_c {
public:
  continue_c() = delete;
  continue_c(std::size_t source_index)
      : base_c(keywords_e::CONTINUE, source_index) {}

  void accept(visitor_if &visitor) const override;
};

class defer_c : public base_c {
public:
  defer_c() = delete;
  defer_c(std::size_t source_index, base_ptr deferred_code)
      : base_c(keywords_e::DEFER, source_index),
        _deferred_code(std::move(deferred_code)) {}

  const base_c *deferred_code() const { return _deferred_code.get(); }

  void accept(visitor_if &visitor) const override;

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

private:
  type_ptr _type;
};

enum class literal_type_e { INTEGER, FLOAT, STRING, BOOL, NIL };

class literal_c : public base_c {
public:
  literal_c() = delete;
  literal_c(std::size_t source_index, literal_type_e type, std::string value)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index), _type(type),
        _value(std::move(value)) {}

  literal_type_e type() const { return _type; }
  const std::string &value() const { return _value; }

  void accept(visitor_if &visitor) const override;

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
                   std::vector<field_initializer_s> field_initializers)
      : base_c(keywords_e::UNKNOWN_KEYWORD, source_index),
        _struct_name(std::move(struct_name)),
        _field_initializers(std::move(field_initializers)) {}

  const identifier_s &struct_name() const { return _struct_name; }
  const std::vector<field_initializer_s> &field_initializers() const {
    return _field_initializers;
  }

  void accept(visitor_if &visitor) const override;

private:
  identifier_s _struct_name;
  std::vector<field_initializer_s> _field_initializers;
};

} // namespace truk::language::nodes