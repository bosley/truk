#include "language/keywords.hpp"
#include <unordered_map>

namespace truk::language {

static const std::unordered_map<std::string, keywords_e> string_to_keyword = {
    {"fn", keywords_e::FN},           {"struct", keywords_e::STRUCT},
    {"var", keywords_e::VAR},         {"const", keywords_e::CONST},
    {"if", keywords_e::IF},           {"else", keywords_e::ELSE},
    {"while", keywords_e::WHILE},     {"for", keywords_e::FOR},
    {"in", keywords_e::IN},           {"return", keywords_e::RETURN},
    {"break", keywords_e::BREAK},     {"continue", keywords_e::CONTINUE},
    {"defer", keywords_e::DEFER},     {"as", keywords_e::AS},
    {"true", keywords_e::TRUE},       {"false", keywords_e::FALSE},
    {"nil", keywords_e::NIL},         {"import", keywords_e::IMPORT},
    {"cimport", keywords_e::CIMPORT}, {"extern", keywords_e::EXTERN},
    {"i8", keywords_e::I8},           {"i16", keywords_e::I16},
    {"i32", keywords_e::I32},         {"i64", keywords_e::I64},
    {"u8", keywords_e::U8},           {"u16", keywords_e::U16},
    {"u32", keywords_e::U32},         {"u64", keywords_e::U64},
    {"f32", keywords_e::F32},         {"f64", keywords_e::F64},
    {"bool", keywords_e::BOOL},       {"void", keywords_e::VOID}};

static const std::unordered_map<keywords_e, std::string> keyword_to_string = {
    {keywords_e::FN, "fn"},           {keywords_e::STRUCT, "struct"},
    {keywords_e::VAR, "var"},         {keywords_e::CONST, "const"},
    {keywords_e::IF, "if"},           {keywords_e::ELSE, "else"},
    {keywords_e::WHILE, "while"},     {keywords_e::FOR, "for"},
    {keywords_e::IN, "in"},           {keywords_e::RETURN, "return"},
    {keywords_e::BREAK, "break"},     {keywords_e::CONTINUE, "continue"},
    {keywords_e::DEFER, "defer"},     {keywords_e::AS, "as"},
    {keywords_e::TRUE, "true"},       {keywords_e::FALSE, "false"},
    {keywords_e::NIL, "nil"},         {keywords_e::IMPORT, "import"},
    {keywords_e::CIMPORT, "cimport"}, {keywords_e::EXTERN, "extern"},
    {keywords_e::I8, "i8"},           {keywords_e::I16, "i16"},
    {keywords_e::I32, "i32"},         {keywords_e::I64, "i64"},
    {keywords_e::U8, "u8"},           {keywords_e::U16, "u16"},
    {keywords_e::U32, "u32"},         {keywords_e::U64, "u64"},
    {keywords_e::F32, "f32"},         {keywords_e::F64, "f64"},
    {keywords_e::BOOL, "bool"},       {keywords_e::VOID, "void"}};

std::optional<keywords_e> keywords_c::from_string(const std::string &str) {
  auto it = string_to_keyword.find(str);
  if (it != string_to_keyword.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::string keywords_c::to_string(keywords_e keyword) {
  auto it = keyword_to_string.find(keyword);
  if (it != keyword_to_string.end()) {
    return it->second;
  }
  return "";
}

} // namespace truk::language
