#include "truk/core/rll.hpp"

#include "rll/RLL.h"

namespace truk::core::rll {

rll_wrapper_c::rll_wrapper_c() : lib_(new ::rll::shared_library()) {}

rll_wrapper_c::~rll_wrapper_c() = default;

rll_wrapper_c::rll_wrapper_c(rll_wrapper_c &&) noexcept = default;

rll_wrapper_c &rll_wrapper_c::operator=(rll_wrapper_c &&) noexcept = default;

void rll_wrapper_c::load(const std::string &path) {
  try {
    lib_->load(path);
  } catch (::rll::exception::library_loading_error &e) {
    throw rll_exception_c(static_cast<int>(rll_error_e::LIBRARY_LOADING_ERROR),
                          e.what());
  } catch (::rll::exception::library_already_loaded &e) {
    throw rll_exception_c(static_cast<int>(rll_error_e::LIBRARY_ALREADY_LOADED),
                          e.what());
  }
}

void rll_wrapper_c::unload() { lib_->unload(); }

bool rll_wrapper_c::is_loaded() const { return lib_->is_loaded(); }

bool rll_wrapper_c::has_symbol(const std::string &symbol) const {
  return lib_->has_symbol(symbol);
}

void *rll_wrapper_c::get_symbol(const std::string &symbol) {
  try {
    return lib_->get_symbol(symbol);
  } catch (::rll::exception::symbol_not_found &e) {
    throw rll_exception_c(static_cast<int>(rll_error_e::SYMBOL_NOT_FOUND),
                          e.what());
  } catch (::rll::exception::library_not_loaded &e) {
    throw rll_exception_c(static_cast<int>(rll_error_e::LIBRARY_NOT_LOADED),
                          "Library not loaded");
  }
}

const std::string &rll_wrapper_c::get_path() const { return lib_->get_path(); }

std::string rll_wrapper_c::get_platform_suffix() {
  return ::rll::shared_library::get_platform_suffix();
}

} // namespace truk::core::rll
