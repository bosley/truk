#pragma once

#include <truk/ingestion/tokenize.hpp>
#include <vector>

namespace truk::ingestion {

class parser_c {
public:
  parser_c() = delete;
  parser_c(const char *data, std::size_t len);
  ~parser_c();

  std::vector<token_s> tokenize();

private:
  const char *_data{nullptr};
  std::size_t _len{0};
};

} // namespace truk::ingestion
