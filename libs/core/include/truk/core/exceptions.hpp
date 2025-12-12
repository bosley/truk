#pragma once

#include <exception>
#include <string>

namespace truk::core {

class truk_exception_c : public std::exception {
public:
  truk_exception_c(const std::string &component, const std::string &message)
      : _component(component), _error_code(0), _message(message),
        _formatted_message("[" + component + "] " + message) {}

  truk_exception_c(const std::string &component, int error_code,
                   const std::string &message)
      : _component(component), _error_code(error_code), _message(message),
        _formatted_message("[" + component + ":" + std::to_string(error_code) +
                           "] " + message) {}

  const char *what() const noexcept override {
    return _formatted_message.c_str();
  }

  const std::string &get_component() const noexcept { return _component; }

  const std::string &get_message() const noexcept { return _message; }

  int get_error_code() const noexcept { return _error_code; }

  virtual ~truk_exception_c() = default;

private:
  std::string _component;
  int _error_code;
  std::string _message;
  std::string _formatted_message;
};

class host_exception_c : public truk_exception_c {
public:
  host_exception_c(int error_code, const std::string &message)
      : truk_exception_c("host", error_code, message) {}
};

class memory_exception_c : public truk_exception_c {
public:
  memory_exception_c(int error_code, const std::string &message)
      : truk_exception_c("memory", error_code, message) {}
};

class environment_exception_c : public truk_exception_c {
public:
  environment_exception_c(int error_code, const std::string &message)
      : truk_exception_c("environment", error_code, message) {}
};

} // namespace truk::core
