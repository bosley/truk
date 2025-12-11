// clang-format off
#include <cstring>
#include <string>
#include <truk/ingestion/parser.hpp>
#include <truk/ingestion/tokenize.hpp>
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
// clang-format on

using namespace truk::ingestion;
using namespace truk::language;

struct parse_result_wrapper_s {
  parse_result_s result;
  parser_c *parser;

  parse_result_wrapper_s(const char *source) {
    parser = new parser_c(source, strlen(source));
    result = parser->parse();
  }

  ~parse_result_wrapper_s() { delete parser; }
};

std::string get_type_name(const nodes::type_c *type) {
  if (!type)
    return "";
  if (auto *prim = dynamic_cast<const nodes::primitive_type_c *>(type)) {
    return truk::language::keywords_c::to_string(prim->keyword());
  }
  if (auto *named = dynamic_cast<const nodes::named_type_c *>(type)) {
    return named->name().name;
  }
  if (auto *ptr = dynamic_cast<const nodes::pointer_type_c *>(type)) {
    return get_type_name(ptr->pointee_type());
  }
  if (auto *arr = dynamic_cast<const nodes::array_type_c *>(type)) {
    return get_type_name(arr->element_type());
  }
  return "";
}

bool is_primitive_type(const nodes::type_c *type, const std::string &name) {
  return get_type_name(type) == name &&
         dynamic_cast<const nodes::primitive_type_c *>(type) != nullptr;
}

bool is_pointer_type(const nodes::type_c *type) {
  return dynamic_cast<const nodes::pointer_type_c *>(type) != nullptr;
}

bool is_array_type(const nodes::type_c *type) {
  return dynamic_cast<const nodes::array_type_c *>(type) != nullptr;
}

std::size_t get_pointer_depth(const nodes::type_c *type) {
  if (auto *ptr = dynamic_cast<const nodes::pointer_type_c *>(type)) {
    return 1 + get_pointer_depth(ptr->pointee_type());
  }
  return 0;
}

std::optional<std::size_t> get_array_size(const nodes::type_c *type) {
  if (auto *arr = dynamic_cast<const nodes::array_type_c *>(type)) {
    return arr->size();
  }
  return std::nullopt;
}

void validate_parse_success(const char *source) {
  parse_result_wrapper_s wrapper(source);
  if (!wrapper.result.success) {
    FAIL(wrapper.result.error_message.c_str());
  }
  CHECK_TRUE(wrapper.result.success);
}

void validate_parse_failure(const char *source,
                            const char *expected_error_substr = nullptr) {
  parse_result_wrapper_s wrapper(source);
  CHECK_FALSE(wrapper.result.success);
  if (expected_error_substr != nullptr) {
    CHECK_TRUE(wrapper.result.error_message.find(expected_error_substr) !=
               std::string::npos);
  }
}

nodes::base_c *get_declaration(const char *source, size_t index = 0) {
  parse_result_wrapper_s wrapper(source);
  if (!wrapper.result.success) {
    FAIL(wrapper.result.error_message.c_str());
  }
  if (index >= wrapper.result.declarations.size()) {
    FAIL("Declaration index out of bounds");
  }
  return wrapper.result.declarations[index].get();
}

TEST_GROUP(ParserFunctionDeclarations){void setup() override{} void teardown()
                                           override{}};

TEST(ParserFunctionDeclarations, EmptyFunction) {
  const char *source = "fn main() {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("main", fn->name().name.c_str());
  CHECK_EQUAL(0, fn->params().size());
  CHECK_TRUE(is_primitive_type(fn->return_type(), "void"));

  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  CHECK_TRUE(body != nullptr);
  CHECK_EQUAL(0, body->statements().size());
}

TEST(ParserFunctionDeclarations, FunctionWithSingleParameter) {
  const char *source = "fn increment(x: i32) {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("increment", fn->name().name.c_str());
  CHECK_EQUAL(1, fn->params().size());

  const auto &param = fn->params()[0];
  STRCMP_EQUAL("x", param.name.name.c_str());
  CHECK_TRUE(is_primitive_type(param.type.get(), "i32"));
  CHECK_EQUAL(0, get_pointer_depth(param.type.get()));
  CHECK_FALSE(get_array_size(param.type.get()).has_value());
}

TEST(ParserFunctionDeclarations, FunctionWithMultipleParameters) {
  const char *source = "fn add(x: i32, y: i32, z: f64) {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("add", fn->name().name.c_str());
  CHECK_EQUAL(3, fn->params().size());

  STRCMP_EQUAL("x", fn->params()[0].name.name.c_str());
  CHECK_TRUE(is_primitive_type(fn->params()[0].type.get(), "i32"));

  STRCMP_EQUAL("y", fn->params()[1].name.name.c_str());
  CHECK_TRUE(is_primitive_type(fn->params()[1].type.get(), "i32"));

  STRCMP_EQUAL("z", fn->params()[2].name.name.c_str());
  CHECK_TRUE(is_primitive_type(fn->params()[2].type.get(), "f64"));
}

TEST(ParserFunctionDeclarations, FunctionWithPrimitiveReturnType) {
  const char *source = "fn get_value(): i64 {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("get_value", fn->name().name.c_str());
  CHECK_TRUE(is_primitive_type(fn->return_type(), "i64"));
  CHECK_EQUAL(0, get_pointer_depth(fn->return_type()));
}

TEST(ParserFunctionDeclarations, FunctionWithPointerReturnType) {
  const char *source = "fn get_ptr(): *i32 {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("get_ptr", fn->name().name.c_str());
  STRCMP_EQUAL("i32", get_type_name(fn->return_type()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(fn->return_type()));
}

TEST(ParserFunctionDeclarations, FunctionWithArrayReturnType) {
  const char *source = "fn get_array(): [10]i32 {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("get_array", fn->name().name.c_str());
  STRCMP_EQUAL("i32", get_type_name(fn->return_type()).c_str());
  CHECK_TRUE(get_array_size(fn->return_type()).has_value());
  CHECK_EQUAL(10, get_array_size(fn->return_type()).value());
}

TEST(ParserFunctionDeclarations, FunctionWithCustomTypeReturn) {
  const char *source = "fn create_point(): Point {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("create_point", fn->name().name.c_str());
  STRCMP_EQUAL("Point", get_type_name(fn->return_type()).c_str());
}

TEST(ParserFunctionDeclarations, FunctionWithPointerParameter) {
  const char *source = "fn process(ptr: *i32) {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  CHECK_EQUAL(1, fn->params().size());
  STRCMP_EQUAL("ptr", fn->params()[0].name.name.c_str());
  STRCMP_EQUAL("i32", get_type_name(fn->params()[0].type.get()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(fn->params()[0].type.get()));
}

TEST(ParserFunctionDeclarations, FunctionWithArrayParameter) {
  const char *source = "fn process_array(arr: [5]i32) {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  CHECK_EQUAL(1, fn->params().size());
  STRCMP_EQUAL("arr", fn->params()[0].name.name.c_str());
  STRCMP_EQUAL("i32", get_type_name(fn->params()[0].type.get()).c_str());
  CHECK_TRUE(get_array_size(fn->params()[0].type.get()).has_value());
  CHECK_EQUAL(5, get_array_size(fn->params()[0].type.get()).value());
}

TEST(ParserFunctionDeclarations, FunctionWithBodyStatements) {
  const char *source = "fn test() { var x: i32 = 42; return x; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  CHECK_TRUE(body != nullptr);
  CHECK_EQUAL(2, body->statements().size());

  auto *var_decl =
      dynamic_cast<const nodes::var_c *>(body->statements()[0].get());
  CHECK_TRUE(var_decl != nullptr);
  STRCMP_EQUAL("x", var_decl->name().name.c_str());

  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[1].get());
  CHECK_TRUE(return_stmt != nullptr);
  CHECK_TRUE(return_stmt->expression() != nullptr);
}

TEST(ParserFunctionDeclarations, FunctionWithComplexBody) {
  const char *source = R"(
    fn factorial(n: i32): i32 {
      if n <= 1 {
        return 1;
      }
      return n * factorial(n - 1);
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("factorial", fn->name().name.c_str());
  STRCMP_EQUAL("i32", get_type_name(fn->return_type()).c_str());
  CHECK_EQUAL(1, fn->params().size());

  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  CHECK_TRUE(body != nullptr);
  CHECK_EQUAL(2, body->statements().size());

  auto *if_stmt =
      dynamic_cast<const nodes::if_c *>(body->statements()[0].get());
  CHECK_TRUE(if_stmt != nullptr);
  CHECK_TRUE(if_stmt->condition() != nullptr);
  CHECK_TRUE(if_stmt->then_block() != nullptr);

  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[1].get());
  CHECK_TRUE(return_stmt != nullptr);
}

TEST(ParserFunctionDeclarations, ErrorMissingFunctionName) {
  const char *source = "fn () {}";
  validate_parse_failure(source, "Expected function name");
}

TEST(ParserFunctionDeclarations, ErrorMissingLeftParen) {
  const char *source = "fn test) {}";
  validate_parse_failure(source, "Expected '(' after function name");
}

TEST(ParserFunctionDeclarations, ErrorMissingRightParen) {
  const char *source = "fn test( {}";
  validate_parse_failure(source, "Expected parameter name");
}

TEST(ParserFunctionDeclarations, ErrorMissingBody) {
  const char *source = "fn test()";
  validate_parse_failure(source, "Expected '{'");
}

TEST(ParserFunctionDeclarations, ErrorInvalidParameterSyntax) {
  const char *source = "fn test(x) {}";
  validate_parse_failure(source, "Expected ':' in type annotation");
}

TEST(ParserFunctionDeclarations, ErrorMissingParameterType) {
  const char *source = "fn test(x:) {}";
  validate_parse_failure(source, "Expected type");
}

TEST(ParserFunctionDeclarations, ErrorUnclosedBody) {
  const char *source = "fn test() {";
  validate_parse_failure(source, "Expected '}' after block");
}

TEST(ParserFunctionDeclarations, MultipleFunctions) {
  const char *source = R"(
    fn first() {}
    fn second(x: i32) {}
    fn third(): bool {}
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(3, wrapper.result.declarations.size());

  auto *fn1 = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn1 != nullptr);
  STRCMP_EQUAL("first", fn1->name().name.c_str());

  auto *fn2 = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[1].get());
  CHECK_TRUE(fn2 != nullptr);
  STRCMP_EQUAL("second", fn2->name().name.c_str());
  CHECK_EQUAL(1, fn2->params().size());

  auto *fn3 = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[2].get());
  CHECK_TRUE(fn3 != nullptr);
  STRCMP_EQUAL("third", fn3->name().name.c_str());
  STRCMP_EQUAL("bool", get_type_name(fn3->return_type()).c_str());
}

TEST_GROUP(ParserStructDeclarations){void setup() override{} void teardown()
                                         override{}};

TEST(ParserStructDeclarations, EmptyStruct) {
  const char *source = "struct Point {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);
  STRCMP_EQUAL("Point", struct_decl->name().name.c_str());
  CHECK_EQUAL(0, struct_decl->fields().size());
}

TEST(ParserStructDeclarations, StructWithSingleField) {
  const char *source = "struct Point { x: i32 }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);
  STRCMP_EQUAL("Point", struct_decl->name().name.c_str());
  CHECK_EQUAL(1, struct_decl->fields().size());

  const auto &field = struct_decl->fields()[0];
  STRCMP_EQUAL("x", field.name.name.c_str());
  STRCMP_EQUAL("i32", get_type_name(field.type.get()).c_str());
  CHECK_EQUAL(0, get_pointer_depth(field.type.get()));
  CHECK_FALSE(get_array_size(field.type.get()).has_value());
}

TEST(ParserStructDeclarations, StructWithMultipleFields) {
  const char *source = "struct Point { x: i32, y: i32, z: f64 }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);
  STRCMP_EQUAL("Point", struct_decl->name().name.c_str());
  CHECK_EQUAL(3, struct_decl->fields().size());

  STRCMP_EQUAL("x", struct_decl->fields()[0].name.name.c_str());
  STRCMP_EQUAL("i32", get_type_name(struct_decl->fields()[0].type.get()).c_str());

  STRCMP_EQUAL("y", struct_decl->fields()[1].name.name.c_str());
  STRCMP_EQUAL("i32", get_type_name(struct_decl->fields()[1].type.get()).c_str());

  STRCMP_EQUAL("z", struct_decl->fields()[2].name.name.c_str());
  STRCMP_EQUAL("f64", get_type_name(struct_decl->fields()[2].type.get()).c_str());
}

TEST(ParserStructDeclarations, StructWithPointerField) {
  const char *source = "struct Node { next: *Node }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);
  STRCMP_EQUAL("Node", struct_decl->name().name.c_str());
  CHECK_EQUAL(1, struct_decl->fields().size());

  const auto &field = struct_decl->fields()[0];
  STRCMP_EQUAL("next", field.name.name.c_str());
  STRCMP_EQUAL("Node", get_type_name(field.type.get()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(field.type.get()));
}

TEST(ParserStructDeclarations, StructWithArrayField) {
  const char *source = "struct Buffer { data: [256]u8 }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);
  STRCMP_EQUAL("Buffer", struct_decl->name().name.c_str());
  CHECK_EQUAL(1, struct_decl->fields().size());

  const auto &field = struct_decl->fields()[0];
  STRCMP_EQUAL("data", field.name.name.c_str());
  STRCMP_EQUAL("u8", get_type_name(field.type.get()).c_str());
  CHECK_TRUE(get_array_size(field.type.get()).has_value());
  CHECK_EQUAL(256, get_array_size(field.type.get()).value());
}

TEST(ParserStructDeclarations, StructWithCustomTypeField) {
  const char *source =
      "struct Rectangle { topLeft: Point, bottomRight: Point }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);
  STRCMP_EQUAL("Rectangle", struct_decl->name().name.c_str());
  CHECK_EQUAL(2, struct_decl->fields().size());

  STRCMP_EQUAL("topLeft", struct_decl->fields()[0].name.name.c_str());
  STRCMP_EQUAL("Point", get_type_name(struct_decl->fields()[0].type.get()).c_str());

  STRCMP_EQUAL("bottomRight", struct_decl->fields()[1].name.name.c_str());
  STRCMP_EQUAL("Point", get_type_name(struct_decl->fields()[1].type.get()).c_str());
}

TEST(ParserStructDeclarations, StructWithMixedFieldTypes) {
  const char *source =
      "struct Mixed { id: i32, name: *u8, values: [10]f32, next: *Mixed }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);
  STRCMP_EQUAL("Mixed", struct_decl->name().name.c_str());
  CHECK_EQUAL(4, struct_decl->fields().size());

  STRCMP_EQUAL("id", struct_decl->fields()[0].name.name.c_str());
  STRCMP_EQUAL("i32", get_type_name(struct_decl->fields()[0].type.get()).c_str());
  CHECK_EQUAL(0, get_pointer_depth(struct_decl->fields()[0].type.get()));

  STRCMP_EQUAL("name", struct_decl->fields()[1].name.name.c_str());
  STRCMP_EQUAL("u8", get_type_name(struct_decl->fields()[1].type.get()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(struct_decl->fields()[1].type.get()));

  STRCMP_EQUAL("values", struct_decl->fields()[2].name.name.c_str());
  STRCMP_EQUAL("f32", get_type_name(struct_decl->fields()[2].type.get()).c_str());
  CHECK_TRUE(get_array_size(struct_decl->fields()[2].type.get()).has_value());
  CHECK_EQUAL(10, get_array_size(struct_decl->fields()[2].type.get()).value());

  STRCMP_EQUAL("next", struct_decl->fields()[3].name.name.c_str());
  STRCMP_EQUAL("Mixed", get_type_name(struct_decl->fields()[3].type.get()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(struct_decl->fields()[3].type.get()));
}

TEST(ParserStructDeclarations, ErrorMissingStructName) {
  const char *source = "struct {}";
  validate_parse_failure(source, "Expected struct name");
}

TEST(ParserStructDeclarations, ErrorMissingLeftBrace) {
  const char *source = "struct Point }";
  validate_parse_failure(source, "Expected '{'");
}

TEST(ParserStructDeclarations, ErrorMissingRightBrace) {
  const char *source = "struct Point { x: i32";
  validate_parse_failure(source, "Expected '}'");
}

TEST(ParserStructDeclarations, ErrorInvalidFieldSyntax) {
  const char *source = "struct Point { x }";
  validate_parse_failure(source, "Expected ':'");
}

TEST(ParserStructDeclarations, ErrorMissingFieldType) {
  const char *source = "struct Point { x: }";
  validate_parse_failure(source, "Expected type");
}

TEST_GROUP(ParserVariableDeclarations){void setup() override{} void teardown()
                                           override{}};

TEST(ParserVariableDeclarations, VarWithTypeAndInitializer) {
  const char *source = "fn test() { var x: i32 = 42; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  CHECK_TRUE(body != nullptr);
  CHECK_EQUAL(1, body->statements().size());

  auto *var_decl =
      dynamic_cast<const nodes::var_c *>(body->statements()[0].get());
  CHECK_TRUE(var_decl != nullptr);
  STRCMP_EQUAL("x", var_decl->name().name.c_str());
  STRCMP_EQUAL("i32", get_type_name(var_decl->type()).c_str());
  CHECK_TRUE(var_decl->initializer() != nullptr);
}

TEST(ParserVariableDeclarations, VarWithoutTypeAnnotation) {
  const char *source = "fn test() { var x = 42; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_FALSE(wrapper.result.success);
}

TEST(ParserVariableDeclarations, VarWithPointerType) {
  const char *source = "fn test() { var ptr: *i32 = nil; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *var_decl =
      dynamic_cast<const nodes::var_c *>(body->statements()[0].get());

  CHECK_TRUE(var_decl != nullptr);
  STRCMP_EQUAL("ptr", var_decl->name().name.c_str());
  STRCMP_EQUAL("i32", get_type_name(var_decl->type()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(var_decl->type()));
}

TEST(ParserVariableDeclarations, VarWithArrayType) {
  const char *source = "fn test() { var arr: [10]i32 = nil; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *var_decl =
      dynamic_cast<const nodes::var_c *>(body->statements()[0].get());

  CHECK_TRUE(var_decl != nullptr);
  STRCMP_EQUAL("arr", var_decl->name().name.c_str());
  STRCMP_EQUAL("i32", get_type_name(var_decl->type()).c_str());
  CHECK_TRUE(get_array_size(var_decl->type()).has_value());
  CHECK_EQUAL(10, get_array_size(var_decl->type()).value());
}

TEST(ParserVariableDeclarations, VarWithComplexInitializer) {
  const char *source = "fn test() { var result: i32 = x + y * 2; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *var_decl =
      dynamic_cast<const nodes::var_c *>(body->statements()[0].get());

  CHECK_TRUE(var_decl != nullptr);
  STRCMP_EQUAL("result", var_decl->name().name.c_str());
  CHECK_TRUE(var_decl->initializer() != nullptr);

  auto *init_expr =
      dynamic_cast<const nodes::binary_op_c *>(var_decl->initializer());
  CHECK_TRUE(init_expr != nullptr);
  CHECK_TRUE(init_expr->op() == nodes::binary_op_e::ADD);
}

TEST(ParserVariableDeclarations, ErrorMissingVarName) {
  const char *source = "fn test() { var : i32 = 42; }";
  validate_parse_failure(source, "Expected variable name");
}

TEST(ParserVariableDeclarations, ErrorMissingEquals) {
  const char *source = "fn test() { var x: i32 42; }";
  validate_parse_failure(source, "Expected '='");
}

TEST(ParserVariableDeclarations, ErrorMissingInitializer) {
  const char *source = "fn test() { var x: i32 = ; }";
  validate_parse_failure(source, "Expected expression");
}

TEST(ParserVariableDeclarations, ErrorMissingSemicolon) {
  const char *source = "fn test() { var x: i32 = 42 }";
  validate_parse_failure(source, "Expected ';'");
}

TEST_GROUP(ParserConstantDeclarations){void setup() override{} void teardown()
                                           override{}};

TEST(ParserConstantDeclarations, ConstWithTypeAndValue) {
  const char *source = "fn test() { const PI: f64 = 3.14159; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  CHECK_TRUE(body != nullptr);
  CHECK_EQUAL(1, body->statements().size());

  auto *const_decl =
      dynamic_cast<const nodes::const_c *>(body->statements()[0].get());
  CHECK_TRUE(const_decl != nullptr);
  STRCMP_EQUAL("PI", const_decl->name().name.c_str());
  STRCMP_EQUAL("f64", get_type_name(const_decl->type()).c_str());
  CHECK_TRUE(const_decl->value() != nullptr);
}

TEST(ParserConstantDeclarations, ConstWithoutTypeAnnotation) {
  const char *source = "fn test() { const MAX = 100; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_FALSE(wrapper.result.success);
}

TEST(ParserConstantDeclarations, ConstWithPointerType) {
  const char *source = "fn test() { const ptr: *i32 = nil; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *const_decl =
      dynamic_cast<const nodes::const_c *>(body->statements()[0].get());

  CHECK_TRUE(const_decl != nullptr);
  STRCMP_EQUAL("ptr", const_decl->name().name.c_str());
  STRCMP_EQUAL("i32", get_type_name(const_decl->type()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(const_decl->type()));
}

TEST(ParserConstantDeclarations, ConstWithArrayType) {
  const char *source = "fn test() { const arr: [5]i32 = nil; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *const_decl =
      dynamic_cast<const nodes::const_c *>(body->statements()[0].get());

  CHECK_TRUE(const_decl != nullptr);
  STRCMP_EQUAL("arr", const_decl->name().name.c_str());
  STRCMP_EQUAL("i32", get_type_name(const_decl->type()).c_str());
  CHECK_TRUE(get_array_size(const_decl->type()).has_value());
  CHECK_EQUAL(5, get_array_size(const_decl->type()).value());
}

TEST(ParserConstantDeclarations, ConstWithComplexValue) {
  const char *source = "fn test() { const result: i32 = a + b * 3; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *const_decl =
      dynamic_cast<const nodes::const_c *>(body->statements()[0].get());

  CHECK_TRUE(const_decl != nullptr);
  STRCMP_EQUAL("result", const_decl->name().name.c_str());
  CHECK_TRUE(const_decl->value() != nullptr);

  auto *value_expr =
      dynamic_cast<const nodes::binary_op_c *>(const_decl->value());
  CHECK_TRUE(value_expr != nullptr);
  CHECK_TRUE(value_expr->op() == nodes::binary_op_e::ADD);
}

TEST(ParserConstantDeclarations, ErrorMissingConstName) {
  const char *source = "fn test() { const : i32 = 42; }";
  validate_parse_failure(source, "Expected constant name");
}

TEST(ParserConstantDeclarations, ErrorMissingEquals) {
  const char *source = "fn test() { const X: i32 42; }";
  validate_parse_failure(source, "Expected '='");
}

TEST(ParserConstantDeclarations, ErrorMissingValue) {
  const char *source = "fn test() { const X: i32 = ; }";
  validate_parse_failure(source, "Expected expression");
}

TEST(ParserConstantDeclarations, ErrorMissingSemicolon) {
  const char *source = "fn test() { const X: i32 = 42 }";
  validate_parse_failure(source, "Expected ';'");
}

TEST_GROUP(ParserTypeSystem){void setup() override{} void teardown()
                                 override{}};

TEST(ParserTypeSystem, PrimitiveTypes) {
  const char *source = R"(
    fn test_i8(x: i8): i8 {}
    fn test_i16(x: i16): i16 {}
    fn test_i32(x: i32): i32 {}
    fn test_i64(x: i64): i64 {}
    fn test_u8(x: u8): u8 {}
    fn test_u16(x: u16): u16 {}
    fn test_u32(x: u32): u32 {}
    fn test_u64(x: u64): u64 {}
    fn test_f32(x: f32): f32 {}
    fn test_f64(x: f64): f64 {}
    fn test_bool(x: bool): bool {}
    fn test_void(): void {}
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(12, wrapper.result.declarations.size());

  const char *expected_types[] = {"i8",  "i16", "i32", "i64", "u8",   "u16",
                                  "u32", "u64", "f32", "f64", "bool", "void"};

  for (size_t i = 0; i < 12; i++) {
    auto *fn =
        dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[i].get());
    CHECK_TRUE(fn != nullptr);
    STRCMP_EQUAL(expected_types[i], get_type_name(fn->return_type()).c_str());
    if (i < 11) {
      CHECK_EQUAL(1, fn->params().size());
      STRCMP_EQUAL(expected_types[i], get_type_name(fn->params()[0].type.get()).c_str());
    }
  }
}

TEST(ParserTypeSystem, SinglePointerType) {
  const char *source = "fn test(ptr: *i32): *i32 {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("i32", get_type_name(fn->params()[0].type.get()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(fn->params()[0].type.get()));

  STRCMP_EQUAL("i32", get_type_name(fn->return_type()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(fn->return_type()));
}

TEST(ParserTypeSystem, MultiLevelPointerType) {
  const char *source = "fn test(ptr: **i32): **i32 {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("i32", get_type_name(fn->params()[0].type.get()).c_str());
  CHECK_EQUAL(2, get_pointer_depth(fn->params()[0].type.get()));

  STRCMP_EQUAL("i32", get_type_name(fn->return_type()).c_str());
  CHECK_EQUAL(2, get_pointer_depth(fn->return_type()));
}

TEST(ParserTypeSystem, SizedArrayType) {
  const char *source = "fn test(arr: [10]i32): [5]i32 {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("i32", get_type_name(fn->params()[0].type.get()).c_str());
  CHECK_TRUE(get_array_size(fn->params()[0].type.get()).has_value());
  CHECK_EQUAL(10, get_array_size(fn->params()[0].type.get()).value());

  STRCMP_EQUAL("i32", get_type_name(fn->return_type()).c_str());
  CHECK_TRUE(get_array_size(fn->return_type()).has_value());
  CHECK_EQUAL(5, get_array_size(fn->return_type()).value());
}

TEST(ParserTypeSystem, UnsizedArrayType) {
  const char *source = "fn test(arr: []i32): []i32 {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("i32", get_type_name(fn->params()[0].type.get()).c_str());
  CHECK_FALSE(get_array_size(fn->params()[0].type.get()).has_value());

  STRCMP_EQUAL("i32", get_type_name(fn->return_type()).c_str());
  CHECK_FALSE(get_array_size(fn->return_type()).has_value());
}

TEST(ParserTypeSystem, ArrayOfPointers) {
  const char *source = "fn test(arr: [10]*i32) {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  CHECK_TRUE(is_array_type(fn->params()[0].type.get()));
  CHECK_EQUAL(10, get_array_size(fn->params()[0].type.get()).value());
  auto *arr_type = dynamic_cast<const nodes::array_type_c *>(fn->params()[0].type.get());
  CHECK_TRUE(is_pointer_type(arr_type->element_type()));
  STRCMP_EQUAL("i32", get_type_name(arr_type->element_type()).c_str());
}

TEST(ParserTypeSystem, PointerToArray) {
  const char *source = "fn test(ptr: *[10]i32) {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  CHECK_TRUE(is_pointer_type(fn->params()[0].type.get()));
  CHECK_EQUAL(1, get_pointer_depth(fn->params()[0].type.get()));
  
  auto *ptr_type = dynamic_cast<const nodes::pointer_type_c *>(fn->params()[0].type.get());
  CHECK_TRUE(ptr_type != nullptr);
  CHECK_TRUE(is_array_type(ptr_type->pointee_type()));
  
  auto *arr_type = dynamic_cast<const nodes::array_type_c *>(ptr_type->pointee_type());
  CHECK_TRUE(arr_type != nullptr);
  CHECK_EQUAL(10, arr_type->size().value());
  STRCMP_EQUAL("i32", get_type_name(arr_type->element_type()).c_str());
}

TEST(ParserTypeSystem, PointerToArrayVsArrayOfPointers) {
  const char *source = "fn test(ptr_to_arr: *[5]i32, arr_of_ptr: [5]*i32) {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);
  CHECK_EQUAL(2, fn->params().size());

  auto *ptr_to_arr_type = fn->params()[0].type.get();
  CHECK_TRUE(is_pointer_type(ptr_to_arr_type));
  CHECK_FALSE(is_array_type(ptr_to_arr_type));
  auto *ptr_node = dynamic_cast<const nodes::pointer_type_c *>(ptr_to_arr_type);
  CHECK_TRUE(is_array_type(ptr_node->pointee_type()));
  auto *inner_arr = dynamic_cast<const nodes::array_type_c *>(ptr_node->pointee_type());
  CHECK_EQUAL(5, inner_arr->size().value());
  STRCMP_EQUAL("i32", get_type_name(inner_arr->element_type()).c_str());

  auto *arr_of_ptr_type = fn->params()[1].type.get();
  CHECK_TRUE(is_array_type(arr_of_ptr_type));
  CHECK_FALSE(is_pointer_type(arr_of_ptr_type));
  auto *arr_node = dynamic_cast<const nodes::array_type_c *>(arr_of_ptr_type);
  CHECK_EQUAL(5, arr_node->size().value());
  CHECK_TRUE(is_pointer_type(arr_node->element_type()));
  auto *inner_ptr = dynamic_cast<const nodes::pointer_type_c *>(arr_node->element_type());
  STRCMP_EQUAL("i32", get_type_name(inner_ptr->pointee_type()).c_str());
}

TEST(ParserTypeSystem, NamedCustomType) {
  const char *source = "fn test(p: Point): Point {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("Point", get_type_name(fn->params()[0].type.get()).c_str());
  CHECK_EQUAL(0, get_pointer_depth(fn->params()[0].type.get()));

  STRCMP_EQUAL("Point", get_type_name(fn->return_type()).c_str());
  CHECK_EQUAL(0, get_pointer_depth(fn->return_type()));
}

TEST(ParserTypeSystem, PointerToCustomType) {
  const char *source = "fn test(p: *Point): *Point {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("Point", get_type_name(fn->params()[0].type.get()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(fn->params()[0].type.get()));

  STRCMP_EQUAL("Point", get_type_name(fn->return_type()).c_str());
  CHECK_EQUAL(1, get_pointer_depth(fn->return_type()));
}

TEST(ParserTypeSystem, ArrayOfCustomType) {
  const char *source = "fn test(arr: [5]Point): [10]Point {}";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  STRCMP_EQUAL("Point", get_type_name(fn->params()[0].type.get()).c_str());
  CHECK_TRUE(get_array_size(fn->params()[0].type.get()).has_value());
  CHECK_EQUAL(5, get_array_size(fn->params()[0].type.get()).value());

  STRCMP_EQUAL("Point", get_type_name(fn->return_type()).c_str());
  CHECK_TRUE(get_array_size(fn->return_type()).has_value());
  CHECK_EQUAL(10, get_array_size(fn->return_type()).value());
}

TEST(ParserTypeSystem, ErrorInvalidTypeSyntax) {
  const char *source = "fn test(x: @invalid) {}";
  validate_parse_failure(source, "Expected type");
}

TEST(ParserTypeSystem, ErrorMissingArraySize) {
  const char *source = "fn test(arr: [)i32) {}";
  validate_parse_failure(source);
}

TEST_GROUP(ParserControlFlow){void setup() override{} void teardown()
                                  override{}};

TEST(ParserControlFlow, SimpleIfStatement) {
  const char *source = "fn test() { if x > 0 { return 1; } }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *if_stmt =
      dynamic_cast<const nodes::if_c *>(body->statements()[0].get());

  CHECK_TRUE(if_stmt != nullptr);
  CHECK_TRUE(if_stmt->condition() != nullptr);
  CHECK_TRUE(if_stmt->then_block() != nullptr);
  CHECK_FALSE(if_stmt->else_block() != nullptr);
}

TEST(ParserControlFlow, IfElseStatement) {
  const char *source =
      "fn test() { if x > 0 { return 1; } else { return 0; } }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *if_stmt =
      dynamic_cast<const nodes::if_c *>(body->statements()[0].get());

  CHECK_TRUE(if_stmt != nullptr);
  CHECK_TRUE(if_stmt->condition() != nullptr);
  CHECK_TRUE(if_stmt->then_block() != nullptr);
  CHECK_TRUE(if_stmt->else_block() != nullptr);
}

TEST(ParserControlFlow, IfElseIfElseChain) {
  const char *source = R"(
    fn test() {
      if x > 0 {
        return 1;
      } else if x < 0 {
        return -1;
      } else {
        return 0;
      }
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *if_stmt =
      dynamic_cast<const nodes::if_c *>(body->statements()[0].get());

  CHECK_TRUE(if_stmt != nullptr);
  CHECK_TRUE(if_stmt->condition() != nullptr);
  CHECK_TRUE(if_stmt->then_block() != nullptr);
  CHECK_TRUE(if_stmt->else_block() != nullptr);

  auto *else_if = dynamic_cast<const nodes::if_c *>(if_stmt->else_block());
  CHECK_TRUE(else_if != nullptr);
  CHECK_TRUE(else_if->condition() != nullptr);
  CHECK_TRUE(else_if->else_block() != nullptr);
}

TEST(ParserControlFlow, NestedIfStatements) {
  const char *source = R"(
    fn test() {
      if x > 0 {
        if y > 0 {
          return 1;
        }
      }
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *outer_if =
      dynamic_cast<const nodes::if_c *>(body->statements()[0].get());

  CHECK_TRUE(outer_if != nullptr);

  auto *then_block =
      dynamic_cast<const nodes::block_c *>(outer_if->then_block());
  CHECK_TRUE(then_block != nullptr);

  auto *inner_if =
      dynamic_cast<const nodes::if_c *>(then_block->statements()[0].get());
  CHECK_TRUE(inner_if != nullptr);
}

TEST(ParserControlFlow, WhileLoop) {
  const char *source = "fn test() { while x > 0 { x = x - 1; } }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *while_stmt =
      dynamic_cast<const nodes::while_c *>(body->statements()[0].get());

  CHECK_TRUE(while_stmt != nullptr);
  CHECK_TRUE(while_stmt->condition() != nullptr);
  CHECK_TRUE(while_stmt->body() != nullptr);
}

TEST(ParserControlFlow, WhileLoopWithBreak) {
  const char *source = "fn test() { while true { break; } }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *while_stmt =
      dynamic_cast<const nodes::while_c *>(body->statements()[0].get());

  CHECK_TRUE(while_stmt != nullptr);

  auto *loop_body = dynamic_cast<const nodes::block_c *>(while_stmt->body());
  CHECK_TRUE(loop_body != nullptr);

  auto *break_stmt =
      dynamic_cast<const nodes::break_c *>(loop_body->statements()[0].get());
  CHECK_TRUE(break_stmt != nullptr);
}

TEST(ParserControlFlow, WhileLoopWithContinue) {
  const char *source = "fn test() { while true { continue; } }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *while_stmt =
      dynamic_cast<const nodes::while_c *>(body->statements()[0].get());

  CHECK_TRUE(while_stmt != nullptr);

  auto *loop_body = dynamic_cast<const nodes::block_c *>(while_stmt->body());
  CHECK_TRUE(loop_body != nullptr);

  auto *continue_stmt =
      dynamic_cast<const nodes::continue_c *>(loop_body->statements()[0].get());
  CHECK_TRUE(continue_stmt != nullptr);
}

TEST(ParserControlFlow, ForLoopCStyle) {
  const char *source =
      "fn test() { for i = 0; i < 10; i = i + 1 { x = x + i; } }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *for_stmt =
      dynamic_cast<const nodes::for_c *>(body->statements()[0].get());

  CHECK_TRUE(for_stmt != nullptr);
  CHECK_TRUE(for_stmt->init() != nullptr);
  CHECK_TRUE(for_stmt->condition() != nullptr);
  CHECK_TRUE(for_stmt->post() != nullptr);
  CHECK_TRUE(for_stmt->body() != nullptr);
}

TEST(ParserControlFlow, ForLoopWithBreakContinue) {
  const char *source = R"(
    fn test() {
      for i = 0; i < 10; i = i + 1 {
        if i == 5 { break; }
        if i == 3 { continue; }
      }
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *for_stmt =
      dynamic_cast<const nodes::for_c *>(body->statements()[0].get());

  CHECK_TRUE(for_stmt != nullptr);
  CHECK_TRUE(for_stmt->body() != nullptr);
}

TEST(ParserControlFlow, NestedLoops) {
  const char *source = R"(
    fn test() {
      while x > 0 {
        for i = 0; i < 10; i = i + 1 {
          x = x - 1;
        }
      }
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *while_stmt =
      dynamic_cast<const nodes::while_c *>(body->statements()[0].get());

  CHECK_TRUE(while_stmt != nullptr);

  auto *while_body = dynamic_cast<const nodes::block_c *>(while_stmt->body());
  CHECK_TRUE(while_body != nullptr);

  auto *for_stmt =
      dynamic_cast<const nodes::for_c *>(while_body->statements()[0].get());
  CHECK_TRUE(for_stmt != nullptr);
}

TEST(ParserControlFlow, ReturnStatement) {
  const char *source = "fn test() { return; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());

  CHECK_TRUE(return_stmt != nullptr);
  CHECK_FALSE(return_stmt->expression() != nullptr);
}

TEST(ParserControlFlow, ReturnWithExpression) {
  const char *source = "fn test(): i32 { return 42; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());

  CHECK_TRUE(return_stmt != nullptr);
  CHECK_TRUE(return_stmt->expression() != nullptr);
}

TEST(ParserControlFlow, ErrorMissingIfCondition) {
  const char *source = "fn test() { if { return 1; } }";
  validate_parse_failure(source, "Expected expression");
}

TEST(ParserControlFlow, ErrorMissingIfBody) {
  const char *source = "fn test() { if x > 0 }";
  validate_parse_failure(source, "Expected '{'");
}

TEST(ParserControlFlow, ErrorMissingWhileCondition) {
  const char *source = "fn test() { while { x = x - 1; } }";
  validate_parse_failure(source, "Expected expression");
}

TEST(ParserControlFlow, ErrorMissingWhileBody) {
  const char *source = "fn test() { while x > 0 }";
  validate_parse_failure(source, "Expected '{'");
}

TEST(ParserControlFlow, ErrorInvalidForSyntax) {
  const char *source = "fn test() { for { x = x + 1; } }";
  validate_parse_failure(source);
}

TEST_GROUP(ParserExpressions){void setup() override{} void teardown()
                                  override{}};

TEST(ParserExpressions, BinaryAddition) {
  const char *source = "fn test() { return a + b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::ADD);
  CHECK_TRUE(bin_op->left() != nullptr);
  CHECK_TRUE(bin_op->right() != nullptr);
}

TEST(ParserExpressions, BinarySubtraction) {
  const char *source = "fn test() { return a - b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::SUB);
}

TEST(ParserExpressions, BinaryMultiplication) {
  const char *source = "fn test() { return a * b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::MUL);
}

TEST(ParserExpressions, BinaryDivision) {
  const char *source = "fn test() { return a / b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::DIV);
}

TEST(ParserExpressions, BinaryModulo) {
  const char *source = "fn test() { return a % b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::MOD);
}

TEST(ParserExpressions, LogicalAnd) {
  const char *source = "fn test() { return a && b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::AND);
}

TEST(ParserExpressions, LogicalOr) {
  const char *source = "fn test() { return a || b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::OR);
}

TEST(ParserExpressions, BitwiseAnd) {
  const char *source = "fn test() { return a & b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::BITWISE_AND);
}

TEST(ParserExpressions, BitwiseOr) {
  const char *source = "fn test() { return a | b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::BITWISE_OR);
}

TEST(ParserExpressions, BitwiseXor) {
  const char *source = "fn test() { return a ^ b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::BITWISE_XOR);
}

TEST(ParserExpressions, LeftShift) {
  const char *source = "fn test() { return a << b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::LEFT_SHIFT);
}

TEST(ParserExpressions, RightShift) {
  const char *source = "fn test() { return a >> b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::RIGHT_SHIFT);
}

TEST(ParserExpressions, EqualityComparison) {
  const char *source = "fn test() { return a == b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::EQ);
}

TEST(ParserExpressions, InequalityComparison) {
  const char *source = "fn test() { return a != b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::NE);
}

TEST(ParserExpressions, LessThanComparison) {
  const char *source = "fn test() { return a < b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::LT);
}

TEST(ParserExpressions, LessEqualComparison) {
  const char *source = "fn test() { return a <= b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::LE);
}

TEST(ParserExpressions, GreaterThanComparison) {
  const char *source = "fn test() { return a > b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::GT);
}

TEST(ParserExpressions, GreaterEqualComparison) {
  const char *source = "fn test() { return a >= b; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *bin_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(bin_op != nullptr);
  CHECK_TRUE(bin_op->op() == nodes::binary_op_e::GE);
}

TEST(ParserExpressions, UnaryNegation) {
  const char *source = "fn test() { return -x; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *unary_op =
      dynamic_cast<const nodes::unary_op_c *>(return_stmt->expression());

  CHECK_TRUE(unary_op != nullptr);
  CHECK_TRUE(unary_op->op() == nodes::unary_op_e::NEG);
  CHECK_TRUE(unary_op->operand() != nullptr);
}

TEST(ParserExpressions, UnaryLogicalNot) {
  const char *source = "fn test() { return !x; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *unary_op =
      dynamic_cast<const nodes::unary_op_c *>(return_stmt->expression());

  CHECK_TRUE(unary_op != nullptr);
  CHECK_TRUE(unary_op->op() == nodes::unary_op_e::NOT);
}

TEST(ParserExpressions, UnaryBitwiseNot) {
  const char *source = "fn test() { return ~x; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *unary_op =
      dynamic_cast<const nodes::unary_op_c *>(return_stmt->expression());

  CHECK_TRUE(unary_op != nullptr);
  CHECK_TRUE(unary_op->op() == nodes::unary_op_e::BITWISE_NOT);
}

TEST(ParserExpressions, UnaryAddressOf) {
  const char *source = "fn test() { return &x; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *unary_op =
      dynamic_cast<const nodes::unary_op_c *>(return_stmt->expression());

  CHECK_TRUE(unary_op != nullptr);
  CHECK_TRUE(unary_op->op() == nodes::unary_op_e::ADDRESS_OF);
}

TEST(ParserExpressions, UnaryDereference) {
  const char *source = "fn test() { return *ptr; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *unary_op =
      dynamic_cast<const nodes::unary_op_c *>(return_stmt->expression());

  CHECK_TRUE(unary_op != nullptr);
  CHECK_TRUE(unary_op->op() == nodes::unary_op_e::DEREF);
}

TEST(ParserExpressions, SimpleAssignment) {
  const char *source = "fn test() { x = 42; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *assign =
      dynamic_cast<const nodes::assignment_c *>(body->statements()[0].get());

  CHECK_TRUE(assign != nullptr);
  CHECK_TRUE(assign->target() != nullptr);
  CHECK_TRUE(assign->value() != nullptr);
}

TEST(ParserExpressions, CompoundAssignmentAdd) {
  const char *source = "fn test() { x += 5; }";
  parse_result_wrapper_s wrapper(source);

  if (!wrapper.result.success) {
    printf("Parse error: %s\n", wrapper.result.error_message.c_str());
  }
  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *assign =
      dynamic_cast<const nodes::assignment_c *>(body->statements()[0].get());

  CHECK_TRUE(assign != nullptr);
  CHECK_TRUE(assign->target() != nullptr);
  CHECK_TRUE(assign->value() != nullptr);
}

TEST(ParserExpressions, CompoundAssignmentSubtract) {
  const char *source = "fn test() { x -= 5; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *assign =
      dynamic_cast<const nodes::assignment_c *>(body->statements()[0].get());

  CHECK_TRUE(assign != nullptr);
  CHECK_TRUE(assign->target() != nullptr);
  CHECK_TRUE(assign->value() != nullptr);
}

TEST(ParserExpressions, CompoundAssignmentMultiply) {
  const char *source = "fn test() { x *= 5; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *assign =
      dynamic_cast<const nodes::assignment_c *>(body->statements()[0].get());

  CHECK_TRUE(assign != nullptr);
  CHECK_TRUE(assign->target() != nullptr);
  CHECK_TRUE(assign->value() != nullptr);
}

TEST(ParserExpressions, CompoundAssignmentDivide) {
  const char *source = "fn test() { x /= 5; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *assign =
      dynamic_cast<const nodes::assignment_c *>(body->statements()[0].get());

  CHECK_TRUE(assign != nullptr);
  CHECK_TRUE(assign->target() != nullptr);
  CHECK_TRUE(assign->value() != nullptr);
}

TEST(ParserExpressions, CompoundAssignmentModulo) {
  const char *source = "fn test() { x %= 5; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *assign =
      dynamic_cast<const nodes::assignment_c *>(body->statements()[0].get());

  CHECK_TRUE(assign != nullptr);
  CHECK_TRUE(assign->target() != nullptr);
  CHECK_TRUE(assign->value() != nullptr);
}

TEST(ParserExpressions, CompoundAssignmentDereference) {
  const char *source = "fn test() { *ptr += 5; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *assign =
      dynamic_cast<const nodes::assignment_c *>(body->statements()[0].get());

  CHECK_TRUE(assign != nullptr);
  CHECK_TRUE(assign->target() != nullptr);
  CHECK_TRUE(assign->value() != nullptr);

  auto *target_deref =
      dynamic_cast<const nodes::unary_op_c *>(assign->target());
  CHECK_TRUE(target_deref != nullptr);
  CHECK_TRUE(target_deref->op() == nodes::unary_op_e::DEREF);
}

TEST(ParserExpressions, CompoundAssignmentComplexLvalue) {
  const char *source = "fn test() { *arr[i] += 1; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *assign =
      dynamic_cast<const nodes::assignment_c *>(body->statements()[0].get());

  CHECK_TRUE(assign != nullptr);
  CHECK_TRUE(assign->target() != nullptr);
  CHECK_TRUE(assign->value() != nullptr);
}

TEST(ParserExpressions, OperatorPrecedenceArithmetic) {
  const char *source = "fn test() { return a + b * c; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *add_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(add_op != nullptr);
  CHECK_TRUE(add_op->op() == nodes::binary_op_e::ADD);

  auto *mul_op = dynamic_cast<const nodes::binary_op_c *>(add_op->right());
  CHECK_TRUE(mul_op != nullptr);
  CHECK_TRUE(mul_op->op() == nodes::binary_op_e::MUL);
}

TEST(ParserExpressions, OperatorPrecedenceLogical) {
  const char *source = "fn test() { return a || b && c; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *or_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(or_op != nullptr);
  CHECK_TRUE(or_op->op() == nodes::binary_op_e::OR);

  auto *and_op = dynamic_cast<const nodes::binary_op_c *>(or_op->right());
  CHECK_TRUE(and_op != nullptr);
  CHECK_TRUE(and_op->op() == nodes::binary_op_e::AND);
}

TEST(ParserExpressions, OperatorPrecedenceMixed) {
  const char *source = "fn test() { return a + b < c * d; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *lt_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(lt_op != nullptr);
  CHECK_TRUE(lt_op->op() == nodes::binary_op_e::LT);

  auto *add_op = dynamic_cast<const nodes::binary_op_c *>(lt_op->left());
  CHECK_TRUE(add_op != nullptr);
  CHECK_TRUE(add_op->op() == nodes::binary_op_e::ADD);

  auto *mul_op = dynamic_cast<const nodes::binary_op_c *>(lt_op->right());
  CHECK_TRUE(mul_op != nullptr);
  CHECK_TRUE(mul_op->op() == nodes::binary_op_e::MUL);
}

TEST(ParserExpressions, ParenthesizedExpression) {
  const char *source = "fn test() { return (a + b) * c; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *mul_op =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());

  CHECK_TRUE(mul_op != nullptr);
  CHECK_TRUE(mul_op->op() == nodes::binary_op_e::MUL);

  auto *add_op = dynamic_cast<const nodes::binary_op_c *>(mul_op->left());
  CHECK_TRUE(add_op != nullptr);
  CHECK_TRUE(add_op->op() == nodes::binary_op_e::ADD);
}

TEST(ParserExpressions, ComplexNestedExpression) {
  const char *source = "fn test() { return (a + b * c) / (d - e) + f; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());

  CHECK_TRUE(return_stmt != nullptr);
  CHECK_TRUE(return_stmt->expression() != nullptr);

  auto *outer_add =
      dynamic_cast<const nodes::binary_op_c *>(return_stmt->expression());
  CHECK_TRUE(outer_add != nullptr);
  CHECK_TRUE(outer_add->op() == nodes::binary_op_e::ADD);
}

TEST(ParserExpressions, ErrorUnbalancedParentheses) {
  const char *source = "fn test() { return (a + b; }";
  validate_parse_failure(source, "Expected ')'");
}

TEST(ParserExpressions, ErrorInvalidOperator) {
  const char *source = "fn test() { return a @ b; }";
  validate_parse_failure(source);
}

TEST(ParserExpressions, ErrorMissingOperand) {
  const char *source = "fn test() { return a + ; }";
  validate_parse_failure(source, "Expected expression");
}

TEST_GROUP(ParserPostfixOperations){void setup() override{} void teardown()
                                        override{}};

TEST(ParserPostfixOperations, FunctionCallNoArgs) {
  const char *source = "fn test() { return foo(); }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *call = dynamic_cast<const nodes::call_c *>(return_stmt->expression());

  CHECK_TRUE(call != nullptr);
  CHECK_TRUE(call->callee() != nullptr);
  CHECK_EQUAL(0, call->arguments().size());
}

TEST(ParserPostfixOperations, FunctionCallSingleArg) {
  const char *source = "fn test() { return foo(42); }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *call = dynamic_cast<const nodes::call_c *>(return_stmt->expression());

  CHECK_TRUE(call != nullptr);
  CHECK_EQUAL(1, call->arguments().size());
}

TEST(ParserPostfixOperations, FunctionCallMultipleArgs) {
  const char *source = "fn test() { return foo(1, 2, 3); }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *call = dynamic_cast<const nodes::call_c *>(return_stmt->expression());

  CHECK_TRUE(call != nullptr);
  CHECK_EQUAL(3, call->arguments().size());
}

TEST(ParserPostfixOperations, NestedFunctionCalls) {
  const char *source = "fn test() { return foo(bar(baz())); }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *outer_call =
      dynamic_cast<const nodes::call_c *>(return_stmt->expression());

  CHECK_TRUE(outer_call != nullptr);
  CHECK_EQUAL(1, outer_call->arguments().size());

  auto *inner_call =
      dynamic_cast<const nodes::call_c *>(outer_call->arguments()[0].get());
  CHECK_TRUE(inner_call != nullptr);
}

TEST(ParserPostfixOperations, ArrayIndexing) {
  const char *source = "fn test() { return arr[0]; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *index = dynamic_cast<const nodes::index_c *>(return_stmt->expression());

  CHECK_TRUE(index != nullptr);
  CHECK_TRUE(index->object() != nullptr);
  CHECK_TRUE(index->index() != nullptr);
}

TEST(ParserPostfixOperations, MultiDimensionalArrayIndexing) {
  const char *source = "fn test() { return matrix[i][j]; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *outer_index =
      dynamic_cast<const nodes::index_c *>(return_stmt->expression());

  CHECK_TRUE(outer_index != nullptr);

  auto *inner_index =
      dynamic_cast<const nodes::index_c *>(outer_index->object());
  CHECK_TRUE(inner_index != nullptr);
}

TEST(ParserPostfixOperations, MemberAccess) {
  const char *source = "fn test() { return obj.field; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *member =
      dynamic_cast<const nodes::member_access_c *>(return_stmt->expression());

  CHECK_TRUE(member != nullptr);
  CHECK_TRUE(member->object() != nullptr);
  STRCMP_EQUAL("field", member->field().name.c_str());
}

TEST(ParserPostfixOperations, ChainedMemberAccess) {
  const char *source = "fn test() { return obj.field1.field2; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *outer_member =
      dynamic_cast<const nodes::member_access_c *>(return_stmt->expression());

  CHECK_TRUE(outer_member != nullptr);
  STRCMP_EQUAL("field2", outer_member->field().name.c_str());

  auto *inner_member =
      dynamic_cast<const nodes::member_access_c *>(outer_member->object());
  CHECK_TRUE(inner_member != nullptr);
  STRCMP_EQUAL("field1", inner_member->field().name.c_str());
}

TEST(ParserPostfixOperations, MemberAccessOnFunctionCall) {
  const char *source = "fn test() { return foo().bar; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *member =
      dynamic_cast<const nodes::member_access_c *>(return_stmt->expression());

  CHECK_TRUE(member != nullptr);
  STRCMP_EQUAL("bar", member->field().name.c_str());

  auto *call = dynamic_cast<const nodes::call_c *>(member->object());
  CHECK_TRUE(call != nullptr);
}

TEST(ParserPostfixOperations, ArrayIndexOnMemberAccess) {
  const char *source = "fn test() { return obj.arr[0]; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *index = dynamic_cast<const nodes::index_c *>(return_stmt->expression());

  CHECK_TRUE(index != nullptr);

  auto *member = dynamic_cast<const nodes::member_access_c *>(index->object());
  CHECK_TRUE(member != nullptr);
  STRCMP_EQUAL("arr", member->field().name.c_str());
}

TEST(ParserPostfixOperations, ComplexChainedOperations) {
  const char *source = "fn test() { return obj.method(arg).field[0]; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());

  CHECK_TRUE(return_stmt != nullptr);
  CHECK_TRUE(return_stmt->expression() != nullptr);

  auto *index = dynamic_cast<const nodes::index_c *>(return_stmt->expression());
  CHECK_TRUE(index != nullptr);
}

TEST(ParserPostfixOperations, ErrorMissingClosingParen) {
  const char *source = "fn test() { return foo(42; }";
  validate_parse_failure(source, "Expected ')'");
}

TEST(ParserPostfixOperations, ErrorMissingClosingBracket) {
  const char *source = "fn test() { return arr[0; }";
  validate_parse_failure(source, "Expected ']'");
}

TEST(ParserPostfixOperations, ErrorMissingMemberName) {
  const char *source = "fn test() { return obj.; }";
  validate_parse_failure(source, "Expected field name");
}

TEST(ParserPostfixOperations, ErrorInvalidArgumentSyntax) {
  const char *source = "fn test() { return foo(,); }";
  validate_parse_failure(source, "Expected expression");
}

TEST_GROUP(ParserLiterals){void setup() override{} void teardown() override{}};

TEST(ParserLiterals, IntegerLiteralDecimal) {
  const char *source = "fn test() { return 42; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::INTEGER);
  STRCMP_EQUAL("42", literal->value().c_str());
}

TEST(ParserLiterals, IntegerLiteralHexadecimal) {
  const char *source = "fn test() { return 0x2A; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::INTEGER);
  STRCMP_EQUAL("0x2A", literal->value().c_str());
}

TEST(ParserLiterals, IntegerLiteralBinary) {
  const char *source = "fn test() { return 0b101010; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::INTEGER);
  STRCMP_EQUAL("0b101010", literal->value().c_str());
}

TEST(ParserLiterals, IntegerLiteralOctal) {
  const char *source = "fn test() { return 0o52; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::INTEGER);
  STRCMP_EQUAL("0o52", literal->value().c_str());
}

TEST(ParserLiterals, FloatLiteralSimple) {
  const char *source = "fn test() { return 3.14; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::FLOAT);
  STRCMP_EQUAL("3.14", literal->value().c_str());
}

TEST(ParserLiterals, FloatLiteralScientific) {
  const char *source = "fn test() { return 1.5e10; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::FLOAT);
  STRCMP_EQUAL("1.5e10", literal->value().c_str());
}

TEST(ParserLiterals, StringLiteralSimple) {
  const char *source = "fn test() { return \"hello\"; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::STRING);
  STRCMP_EQUAL("\"hello\"", literal->value().c_str());
}

TEST(ParserLiterals, StringLiteralWithEscapes) {
  const char *source = "fn test() { return \"hello\\nworld\\t!\"; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::STRING);
}

TEST(ParserLiterals, BoolLiteralTrue) {
  const char *source = "fn test() { return true; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::BOOL);
  STRCMP_EQUAL("true", literal->value().c_str());
}

TEST(ParserLiterals, BoolLiteralFalse) {
  const char *source = "fn test() { return false; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::BOOL);
  STRCMP_EQUAL("false", literal->value().c_str());
}

TEST(ParserLiterals, NilLiteral) {
  const char *source = "fn test() { return nil; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::NIL);
  STRCMP_EQUAL("nil", literal->value().c_str());
}

TEST(ParserLiterals, IdentifierSimple) {
  const char *source = "fn test() { return myVar; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *identifier =
      dynamic_cast<const nodes::identifier_c *>(return_stmt->expression());

  CHECK_TRUE(identifier != nullptr);
  STRCMP_EQUAL("myVar", identifier->id().name.c_str());
}

TEST(ParserLiterals, IdentifierWithUnderscores) {
  const char *source = "fn test() { return my_var_name; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *identifier =
      dynamic_cast<const nodes::identifier_c *>(return_stmt->expression());

  CHECK_TRUE(identifier != nullptr);
  STRCMP_EQUAL("my_var_name", identifier->id().name.c_str());
}

TEST(ParserLiterals, ParenthesizedExpression) {
  const char *source = "fn test() { return (42); }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *literal =
      dynamic_cast<const nodes::literal_c *>(return_stmt->expression());

  CHECK_TRUE(literal != nullptr);
  CHECK_TRUE(literal->type() == nodes::literal_type_e::INTEGER);
}

TEST(ParserLiterals, ArrayLiteralEmpty) {
  const char *source = "fn test() { return []; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *array_lit =
      dynamic_cast<const nodes::array_literal_c *>(return_stmt->expression());

  CHECK_TRUE(array_lit != nullptr);
  CHECK_EQUAL(0, array_lit->elements().size());
}

TEST(ParserLiterals, ArrayLiteralWithElements) {
  const char *source = "fn test() { return [1, 2, 3]; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *array_lit =
      dynamic_cast<const nodes::array_literal_c *>(return_stmt->expression());

  CHECK_TRUE(array_lit != nullptr);
  CHECK_EQUAL(3, array_lit->elements().size());
}

TEST(ParserLiterals, ArrayLiteralNested) {
  const char *source = "fn test() { return [[1, 2], [3, 4]]; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *outer_array =
      dynamic_cast<const nodes::array_literal_c *>(return_stmt->expression());

  CHECK_TRUE(outer_array != nullptr);
  CHECK_EQUAL(2, outer_array->elements().size());
}

TEST(ParserLiterals, StructLiteralEmpty) {
  const char *source = "fn test() { return Point{}; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *struct_lit =
      dynamic_cast<const nodes::struct_literal_c *>(return_stmt->expression());

  CHECK_TRUE(struct_lit != nullptr);
  STRCMP_EQUAL("Point", struct_lit->struct_name().name.c_str());
  CHECK_EQUAL(0, struct_lit->field_initializers().size());
}

TEST(ParserLiterals, StructLiteralWithFields) {
  const char *source = "fn test() { return Point{x: 1, y: 2}; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *struct_lit =
      dynamic_cast<const nodes::struct_literal_c *>(return_stmt->expression());

  CHECK_TRUE(struct_lit != nullptr);
  STRCMP_EQUAL("Point", struct_lit->struct_name().name.c_str());
  CHECK_EQUAL(2, struct_lit->field_initializers().size());
  STRCMP_EQUAL("x",
               struct_lit->field_initializers()[0].field_name.name.c_str());
  STRCMP_EQUAL("y",
               struct_lit->field_initializers()[1].field_name.name.c_str());
}

TEST(ParserLiterals, StructLiteralNested) {
  const char *source = "fn test() { return Rect{topLeft: Point{x: 0, y: 0}}; }";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  auto *return_stmt =
      dynamic_cast<const nodes::return_c *>(body->statements()[0].get());
  auto *outer_struct =
      dynamic_cast<const nodes::struct_literal_c *>(return_stmt->expression());

  CHECK_TRUE(outer_struct != nullptr);
  STRCMP_EQUAL("Rect", outer_struct->struct_name().name.c_str());
}

TEST(ParserLiterals, ErrorUnterminatedString) {
  const char *source = "fn test() { return \"hello; }";
  validate_parse_failure(source);
}

TEST(ParserLiterals, ErrorInvalidArraySyntax) {
  const char *source = "fn test() { return [1, 2,]; }";
  parse_result_wrapper_s wrapper(source);
  CHECK_TRUE(wrapper.result.success);
}

TEST(ParserLiterals, ErrorInvalidStructSyntax) {
  const char *source = "fn test() { return Point{x}; }";
  validate_parse_failure(source, "Expected ':'");
}

TEST_GROUP(ParserComplexPrograms){void setup() override{} void teardown()
                                      override{}};

TEST(ParserComplexPrograms, MultipleDeclarationsMixed) {
  const char *source = R"(
    struct Point { x: i32, y: i32 }
    fn create_point(x: i32, y: i32): Point {}
    const ORIGIN: Point = Point{x: 0, y: 0};
    fn main() {}
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(4, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);

  auto *fn1 = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[1].get());
  CHECK_TRUE(fn1 != nullptr);

  auto *const_decl =
      dynamic_cast<nodes::const_c *>(wrapper.result.declarations[2].get());
  CHECK_TRUE(const_decl != nullptr);

  auto *fn2 = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[3].get());
  CHECK_TRUE(fn2 != nullptr);
}

TEST(ParserComplexPrograms, FunctionWithStructParameter) {
  const char *source = R"(
    struct Vec2 { x: f32, y: f32 }
    fn magnitude(v: Vec2): f32 {
      return v.x * v.x + v.y * v.y;
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(2, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);
  STRCMP_EQUAL("Vec2", struct_decl->name().name.c_str());

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[1].get());
  CHECK_TRUE(fn != nullptr);
  STRCMP_EQUAL("magnitude", fn->name().name.c_str());
  CHECK_EQUAL(1, fn->params().size());
  STRCMP_EQUAL("Vec2", get_type_name(fn->params()[0].type.get()).c_str());
}

TEST(ParserComplexPrograms, StructWithFunctionPointer) {
  const char *source = R"(
    struct Handler {
      callback: *fn(i32): void,
      data: *void
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);
  STRCMP_EQUAL("Handler", struct_decl->name().name.c_str());
  CHECK_EQUAL(2, struct_decl->fields().size());
}

TEST(ParserComplexPrograms, RecursiveFunctionDefinition) {
  const char *source = R"(
    fn factorial(n: i32): i32 {
      if n <= 1 {
        return 1;
      }
      return n * factorial(n - 1);
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);
  STRCMP_EQUAL("factorial", fn->name().name.c_str());

  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  CHECK_TRUE(body != nullptr);
  CHECK_EQUAL(2, body->statements().size());
}

TEST(ParserComplexPrograms, ComplexControlFlowNesting) {
  const char *source = R"(
    fn process(data: [10]i32): i32 {
      var sum: i32 = 0;
      for i = 0; i < 10; i = i + 1 {
        if data[i] > 0 {
          while data[i] > 100 {
            data[i] = data[i] / 2;
          }
          sum = sum + data[i];
        } else {
          continue;
        }
      }
      return sum;
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  CHECK_TRUE(body != nullptr);
  CHECK_EQUAL(3, body->statements().size());
}

TEST(ParserComplexPrograms, MixedDeclarationsAndStatements) {
  const char *source = R"(
    fn test() {
      var x: i32 = 10;
      const MAX: i32 = 100;
      var y: i32 = x + MAX;
      if y > 50 {
        var z: i32 = y * 2;
        return z;
      }
      return y;
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(1, wrapper.result.declarations.size());

  auto *fn = dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(fn != nullptr);

  auto *body = dynamic_cast<const nodes::block_c *>(fn->body());
  CHECK_TRUE(body != nullptr);
  CHECK_EQUAL(5, body->statements().size());
}

TEST(ParserComplexPrograms, RealWorldExample1) {
  const char *source = R"(
    struct Vec2 {
      x: f32,
      y: f32
    }
    
    fn distance(a: Vec2, b: Vec2): f32 {
      var dx: f32 = b.x - a.x;
      var dy: f32 = b.y - a.y;
      return dx * dx + dy * dy;
    }
    
    fn main() {
      var p1: Vec2 = Vec2{x: 0.0, y: 0.0};
      var p2: Vec2 = Vec2{x: 3.0, y: 4.0};
      var dist: f32 = distance(p1, p2);
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(3, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);

  auto *distance_fn =
      dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[1].get());
  CHECK_TRUE(distance_fn != nullptr);

  auto *main_fn =
      dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[2].get());
  CHECK_TRUE(main_fn != nullptr);
}

TEST(ParserComplexPrograms, RealWorldExample2) {
  const char *source = R"(
    struct Node {
      value: i32,
      next: *Node
    }
    
    fn create_node(val: i32): *Node {
      var node: *Node = nil;
      return node;
    }
    
    fn sum_list(head: *Node): i32 {
      var sum: i32 = 0;
      var current: *Node = head;
      while current != nil {
        sum = sum + current.value;
        current = current.next;
      }
      return sum;
    }
  )";
  parse_result_wrapper_s wrapper(source);

  CHECK_TRUE(wrapper.result.success);
  CHECK_EQUAL(3, wrapper.result.declarations.size());

  auto *struct_decl =
      dynamic_cast<nodes::struct_c *>(wrapper.result.declarations[0].get());
  CHECK_TRUE(struct_decl != nullptr);
  STRCMP_EQUAL("Node", struct_decl->name().name.c_str());

  auto *create_fn =
      dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[1].get());
  CHECK_TRUE(create_fn != nullptr);

  auto *sum_fn =
      dynamic_cast<nodes::fn_c *>(wrapper.result.declarations[2].get());
  CHECK_TRUE(sum_fn != nullptr);
}

TEST(ParserComplexPrograms, ErrorInComplexProgram) {
  const char *source = R"(
    struct Point { x: i32, y: i32 }
    fn test() {
      var p: Point = Point{x: 1, y: 2}
      return p.x;
    }
  )";
  validate_parse_failure(source, "Expected ';'");
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
