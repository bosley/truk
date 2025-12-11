#pragma once

#include <memory>

namespace truk::ingestion {

class parser_c {
public:
  parser_c() = delete;
  parser_c(char *data, std::size_t len);
  ~parser_c();

  void initialize();
  void shutdown();

private:
  char *_target_data{nullptr};
  std::size_t _target_len{0};
};

} // namespace truk::parser
