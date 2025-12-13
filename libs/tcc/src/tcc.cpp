#include "truk/tcc/tcc.hpp"
#include <libtcc.h>

namespace truk::tcc {

tcc_compiler_c::tcc_compiler_c() { m_state = tcc_new(); }

tcc_compiler_c::~tcc_compiler_c() {
  if (m_state) {
    tcc_delete(static_cast<TCCState *>(m_state));
  }
}

void tcc_compiler_c::add_include_path(const std::string &path) {
  tcc_add_include_path(static_cast<TCCState *>(m_state), path.c_str());
}

void tcc_compiler_c::add_library_path(const std::string &path) {
  tcc_add_library_path(static_cast<TCCState *>(m_state), path.c_str());
}

void tcc_compiler_c::add_library(const std::string &lib) {
  tcc_add_library(static_cast<TCCState *>(m_state), lib.c_str());
}

void tcc_compiler_c::set_rpath(const std::string &path) {
  std::string rpath_option = "-Wl,-rpath," + path;
  tcc_set_options(static_cast<TCCState *>(m_state), rpath_option.c_str());
}

void tcc_compiler_c::set_output_type(int type) {
  tcc_set_output_type(static_cast<TCCState *>(m_state), type);
}

compile_result_s tcc_compiler_c::compile_file(const std::string &input_file,
                                              const std::string &output_file) {
  compile_result_s result;
  result.success = false;

  TCCState *state = static_cast<TCCState *>(m_state);

  if (tcc_add_file(state, input_file.c_str()) < 0) {
    result.error_message = "Failed to add input file: " + input_file;
    return result;
  }

  if (tcc_output_file(state, output_file.c_str()) < 0) {
    result.error_message = "Failed to write output file: " + output_file;
    return result;
  }

  result.success = true;
  return result;
}

} // namespace truk::tcc
