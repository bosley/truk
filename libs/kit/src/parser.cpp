#include "truk/kit/kit.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace truk::kit {

enum class token_type_e {
  KEYWORD_PROJECT,
  KEYWORD_LIBRARY,
  KEYWORD_APPLICATION,
  IDENTIFIER,
  LBRACE,
  RBRACE,
  EQUALS,
  STRING_VALUE,
  END_OF_FILE
};

struct token_s {
  token_type_e type;
  std::string value;
  std::size_t position;
  std::size_t line;
  std::size_t column;
};

class lexer_c {
public:
  lexer_c(const std::string &source)
      : source_(source), pos_(0), line_(1), column_(1) {}

  token_s next_token() {
    skip_whitespace_and_comments();

    if (pos_ >= source_.size()) {
      return {token_type_e::END_OF_FILE, "", pos_, line_, column_};
    }

    std::size_t start_pos = pos_;
    std::size_t start_line = line_;
    std::size_t start_column = column_;

    char ch = source_[pos_];

    if (ch == '{') {
      advance();
      return {token_type_e::LBRACE, "{", start_pos, start_line, start_column};
    }

    if (ch == '}') {
      advance();
      return {token_type_e::RBRACE, "}", start_pos, start_line, start_column};
    }

    if (ch == '=') {
      advance();
      return {token_type_e::EQUALS, "=", start_pos, start_line, start_column};
    }

    if (ch == '"') {
      return read_quoted_string(start_pos, start_line, start_column);
    }

    if (std::isalpha(ch) || ch == '_' || ch == '/' || ch == '.') {
      return read_identifier_or_path(start_pos, start_line, start_column);
    }

    if (!std::isspace(ch)) {
      return read_value(start_pos, start_line, start_column);
    }

    throw kit_exception_c(exception_e::PARSE_ERROR, pos_,
                          "Unexpected character: " + std::string(1, ch));
  }
  
  std::size_t get_position() const { return pos_; }
  
  void set_position(std::size_t pos) { 
    pos_ = pos;
  }

private:
  void skip_whitespace_and_comments() {
    while (pos_ < source_.size()) {
      if (std::isspace(source_[pos_])) {
        advance();
      } else if (source_[pos_] == '#') {
        while (pos_ < source_.size() && source_[pos_] != '\n') {
          advance();
        }
      } else {
        break;
      }
    }
  }

  void advance() {
    if (pos_ < source_.size()) {
      if (source_[pos_] == '\n') {
        line_++;
        column_ = 1;
      } else {
        column_++;
      }
      pos_++;
    }
  }

  token_s read_identifier_or_path(std::size_t start_pos, std::size_t start_line,
                                  std::size_t start_column) {
    std::string value;

    if (pos_ < source_.size() &&
        (source_[pos_] == '/' || source_[pos_] == '.')) {
      while (pos_ < source_.size() && !std::isspace(source_[pos_]) &&
             source_[pos_] != '{' && source_[pos_] != '}' &&
             source_[pos_] != '=' && source_[pos_] != '#') {
        value += source_[pos_];
        advance();
      }
      return {token_type_e::STRING_VALUE, value, start_pos, start_line,
              start_column};
    }

    while (pos_ < source_.size() &&
           (std::isalnum(source_[pos_]) || source_[pos_] == '_')) {
      value += source_[pos_];
      advance();
    }

    if (pos_ < source_.size() &&
        (source_[pos_] == '/' || source_[pos_] == '.')) {
      while (pos_ < source_.size() && !std::isspace(source_[pos_]) &&
             source_[pos_] != '{' && source_[pos_] != '}' &&
             source_[pos_] != '=' && source_[pos_] != '#') {
        value += source_[pos_];
        advance();
      }
      return {token_type_e::STRING_VALUE, value, start_pos, start_line,
              start_column};
    }

    token_type_e type = token_type_e::IDENTIFIER;
    if (value == "project") {
      type = token_type_e::KEYWORD_PROJECT;
    } else if (value == "library") {
      type = token_type_e::KEYWORD_LIBRARY;
    } else if (value == "application") {
      type = token_type_e::KEYWORD_APPLICATION;
    }

    return {type, value, start_pos, start_line, start_column};
  }

  token_s read_quoted_string(std::size_t start_pos, std::size_t start_line,
                             std::size_t start_column) {
    advance();
    std::string value;
    while (pos_ < source_.size() && source_[pos_] != '"') {
      if (source_[pos_] == '\\' && pos_ + 1 < source_.size()) {
        advance();
        value += source_[pos_];
      } else {
        value += source_[pos_];
      }
      advance();
    }
    if (pos_ >= source_.size()) {
      throw kit_exception_c(exception_e::PARSE_ERROR, start_pos,
                            "Unterminated string literal");
    }
    advance();
    return {token_type_e::STRING_VALUE, value, start_pos, start_line,
            start_column};
  }

  token_s read_value(std::size_t start_pos, std::size_t start_line,
                     std::size_t start_column) {
    std::string value;
    while (pos_ < source_.size() && !std::isspace(source_[pos_]) &&
           source_[pos_] != '{' && source_[pos_] != '}' &&
           source_[pos_] != '=' && source_[pos_] != '#') {
      value += source_[pos_];
      advance();
    }
    return {token_type_e::STRING_VALUE, value, start_pos, start_line,
            start_column};
  }

  std::string source_;
  std::size_t pos_;
  std::size_t line_;
  std::size_t column_;
};

class parser_c {
public:
  parser_c(const std::filesystem::path &kit_path, const std::string &source)
      : kit_path_(kit_path), lexer_(source),
        current_token_(lexer_.next_token()) {}

  kit_config_s parse() {
    kit_config_s config;
    config.kit_file_directory = kit_path_.parent_path();

    while (current_token_.type != token_type_e::END_OF_FILE) {
      if (current_token_.type == token_type_e::KEYWORD_PROJECT) {
        parse_project(config);
      } else if (current_token_.type == token_type_e::KEYWORD_LIBRARY) {
        parse_library(config);
      } else if (current_token_.type == token_type_e::KEYWORD_APPLICATION) {
        parse_application(config);
      } else {
        throw kit_exception_c(
            exception_e::PARSE_ERROR, current_token_.position,
            "Expected 'project', 'library', or 'application'");
      }
    }

    return config;
  }

private:
  void advance() { current_token_ = lexer_.next_token(); }

  void expect(token_type_e type, const std::string &error_msg) {
    if (current_token_.type != type) {
      throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                            error_msg);
    }
    advance();
  }

  void parse_project(kit_config_s &config) {
    advance();
    if (current_token_.type != token_type_e::IDENTIFIER &&
        current_token_.type != token_type_e::STRING_VALUE) {
      throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                            "Expected project name");
    }
    config.project_name = current_token_.value;
    advance();
  }

  void parse_library(kit_config_s &config) {
    advance();
    if (current_token_.type != token_type_e::IDENTIFIER &&
        current_token_.type != token_type_e::STRING_VALUE) {
      throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                            "Expected library name");
    }
    std::string lib_name = current_token_.value;
    advance();

    for (const auto &[name, _] : config.libraries) {
      if (name == lib_name) {
        throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                              "Duplicate library name: " + lib_name);
      }
    }

    expect(token_type_e::LBRACE, "Expected '{' after library name");

    std::string source;
    std::string output;
    std::optional<std::vector<std::string>> depends;
    std::optional<std::string> test;
    std::optional<std::vector<std::string>> include_paths;

    while (current_token_.type != token_type_e::RBRACE) {
      if (current_token_.type == token_type_e::END_OF_FILE) {
        throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                              "Unexpected end of file in library block");
      }
      if (current_token_.type != token_type_e::IDENTIFIER) {
        throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                              "Expected field name, got: " +
                                  current_token_.value);
      }
      std::string field_name = current_token_.value;
      advance();

      expect(token_type_e::EQUALS, "Expected '=' after field name");

      if (field_name == "source") {
        source = read_value_tokens();
      } else if (field_name == "output") {
        output = read_value_tokens();
      } else if (field_name == "depends") {
        depends = read_list_tokens();
      } else if (field_name == "test") {
        test = read_value_tokens();
      } else if (field_name == "include_paths") {
        auto paths = read_list_tokens();
        std::vector<std::string> resolved;
        for (const auto &p : paths) {
          resolved.push_back(
              resolve_path(config.kit_file_directory, p).string());
        }
        include_paths = resolved;
      } else {
        throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                              "Unknown library field: " + field_name);
      }
    }

    expect(token_type_e::RBRACE, "Expected '}' at end of library block");

    if (source.empty()) {
      throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                            "Library '" + lib_name +
                                "' missing required field 'source'");
    }
    if (output.empty()) {
      throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                            "Library '" + lib_name +
                                "' missing required field 'output'");
    }

    std::string resolved_source =
        resolve_path(config.kit_file_directory, source).string();
    std::string resolved_output =
        resolve_path(config.kit_file_directory, output).string();
    std::optional<std::string> resolved_test;
    if (test.has_value()) {
      resolved_test =
          resolve_path(config.kit_file_directory, test.value()).string();
    }

    config.libraries.emplace_back(
        lib_name, target_library_c(resolved_source, resolved_output, depends,
                                    resolved_test, include_paths));
  }

  void parse_application(kit_config_s &config) {
    advance();
    if (current_token_.type != token_type_e::IDENTIFIER &&
        current_token_.type != token_type_e::STRING_VALUE) {
      throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                            "Expected application name");
    }
    std::string app_name = current_token_.value;
    advance();

    for (const auto &[name, _] : config.applications) {
      if (name == app_name) {
        throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                              "Duplicate application name: " + app_name);
      }
    }

    expect(token_type_e::LBRACE, "Expected '{' after application name");

    std::string source;
    std::string output;
    std::optional<std::vector<std::string>> libraries;
    std::optional<std::vector<std::string>> library_paths;
    std::optional<std::vector<std::string>> include_paths;

    while (current_token_.type != token_type_e::RBRACE) {
      if (current_token_.type == token_type_e::END_OF_FILE) {
        throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                              "Unexpected end of file in application block");
      }
      if (current_token_.type != token_type_e::IDENTIFIER) {
        throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                              "Expected field name, got: " +
                                  current_token_.value);
      }
      std::string field_name = current_token_.value;
      advance();

      expect(token_type_e::EQUALS, "Expected '=' after field name");

      if (field_name == "source") {
        source = read_value_tokens();
      } else if (field_name == "output") {
        output = read_value_tokens();
      } else if (field_name == "libraries") {
        libraries = read_list_tokens();
      } else if (field_name == "library_paths") {
        auto paths = read_list_tokens();
        std::vector<std::string> resolved;
        for (const auto &p : paths) {
          resolved.push_back(
              resolve_path(config.kit_file_directory, p).string());
        }
        library_paths = resolved;
      } else if (field_name == "include_paths") {
        auto paths = read_list_tokens();
        std::vector<std::string> resolved;
        for (const auto &p : paths) {
          resolved.push_back(
              resolve_path(config.kit_file_directory, p).string());
        }
        include_paths = resolved;
      } else {
        throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                              "Unknown application field: " + field_name);
      }
    }

    expect(token_type_e::RBRACE, "Expected '}' at end of application block");

    if (source.empty()) {
      throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                            "Application '" + app_name +
                                "' missing required field 'source'");
    }
    if (output.empty()) {
      throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                            "Application '" + app_name +
                                "' missing required field 'output'");
    }

    std::string resolved_source =
        resolve_path(config.kit_file_directory, source).string();
    std::string resolved_output =
        resolve_path(config.kit_file_directory, output).string();

    config.applications.emplace_back(
        app_name,
        target_application_c(resolved_source, resolved_output, libraries,
                             library_paths, include_paths));
  }

  std::string read_value_tokens() {
    if (current_token_.type != token_type_e::STRING_VALUE &&
        current_token_.type != token_type_e::IDENTIFIER) {
      throw kit_exception_c(exception_e::PARSE_ERROR, current_token_.position,
                            "Expected value");
    }

    std::string value = current_token_.value;
    advance();
    return value;
  }

  std::vector<std::string> read_list_tokens() {
    std::vector<std::string> values;
    while (current_token_.type == token_type_e::STRING_VALUE ||
           current_token_.type == token_type_e::IDENTIFIER) {
      
      std::size_t peek_pos = lexer_.get_position();
      token_s peek_token = lexer_.next_token();
      lexer_.set_position(peek_pos);
      
      if (current_token_.type == token_type_e::IDENTIFIER && 
          peek_token.type == token_type_e::EQUALS) {
        break;
      }
      
      values.push_back(current_token_.value);
      advance();
    }
    return values;
  }

  std::filesystem::path kit_path_;
  lexer_c lexer_;
  token_s current_token_;
};

kit_config_s parse_kit_file(const std::filesystem::path &kit_path) {
  std::ifstream file(kit_path);
  if (!file.is_open()) {
    throw kit_exception_c(exception_e::PARSE_ERROR, 0,
                          "Failed to open kit file: " + kit_path.string());
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();

  parser_c parser(kit_path, source);
  return parser.parse();
}

} // namespace truk::kit
