#include <truk/ingestion/parser.hpp>

namespace truk::ingestion {

parser_c::parser_c(const char *data, std::size_t len)
    : _data(data), _len(len) {}

parser_c::~parser_c() = default;

std::vector<token_s> parser_c::tokenize() {
  std::vector<token_s> tokens;
  tokenizer_c tokenizer(_data, _len);

  while (true) {
    auto token_opt = tokenizer.next_token();
    if (!token_opt.has_value()) {
      break;
    }

    auto token = token_opt.value();
    tokens.push_back(token);

    if (token.type == token_type_e::END_OF_FILE) {
      break;
    }
  }

  return tokens;
}

} // namespace truk::ingestion
