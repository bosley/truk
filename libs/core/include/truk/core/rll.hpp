#pragma once

#include "exceptions.hpp"
#include <functional>
#include <memory>
#include <string>

namespace rll {
class shared_library_c;
}

namespace truk::core::rll {

enum class rll_error_e : int {
  LIBRARY_LOADING_ERROR = 1,
  LIBRARY_ALREADY_LOADED = 2,
  LIBRARY_NOT_LOADED = 3,
  SYMBOL_NOT_FOUND = 4
};

class rll_wrapper_c {
public:
  rll_wrapper_c();
  ~rll_wrapper_c();

  rll_wrapper_c(const rll_wrapper_c &) = delete;
  rll_wrapper_c &operator=(const rll_wrapper_c &) = delete;

  rll_wrapper_c(rll_wrapper_c &&) noexcept;
  rll_wrapper_c &operator=(rll_wrapper_c &&) noexcept;

  void load(const std::string &path);

  void unload();

  [[nodiscard]] bool is_loaded() const;

  [[nodiscard]] bool has_symbol(const std::string &symbol) const;

  void *get_symbol(const std::string &symbol);

  template <typename object_type>
  object_type *get_object_symbol(const std::string &name) {
    return static_cast<object_type *>(get_symbol(name));
  }

  template <typename signature>
  std::function<signature> get_function_symbol(const std::string &name) {
    return reinterpret_cast<signature *>(get_symbol(name));
  }

  [[nodiscard]] const std::string &get_path() const;

  [[nodiscard]] static std::string get_platform_suffix();

private:
  std::unique_ptr<::rll::shared_library_c> lib_;
};

} // namespace truk::core::rll
