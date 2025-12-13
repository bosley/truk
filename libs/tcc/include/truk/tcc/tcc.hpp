#pragma once

#include <string>
#include <vector>

namespace truk::tcc {

enum output_type_e {
  OUTPUT_MEMORY = 1,
  OUTPUT_EXE = 2,
  OUTPUT_DLL = 3,
  OUTPUT_OBJ = 4,
  OUTPUT_PREPROCESS = 5
};

struct compile_result_s {
  bool success;
  std::string error_message;
};

struct run_result_s {
  bool success;
  int exit_code;
  std::string error_message;
};

class tcc_compiler_c {
public:
  tcc_compiler_c();
  ~tcc_compiler_c();

  tcc_compiler_c(const tcc_compiler_c &) = delete;
  tcc_compiler_c &operator=(const tcc_compiler_c &) = delete;

  void add_include_path(const std::string &path);
  void add_library_path(const std::string &path);
  void add_library(const std::string &lib);
  void set_rpath(const std::string &path);
  void set_output_type(int type);

  compile_result_s compile_file(const std::string &input_file,
                                const std::string &output_file);

  run_result_s compile_and_run(const std::string &c_source, int argc,
                               char **argv);

private:
  void *m_state;
};

} // namespace truk::tcc
