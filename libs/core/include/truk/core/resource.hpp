#pragma once

#include <cstddef>
#include <limits>

namespace truk::core {

class resource_if {
public:
  static constexpr std::size_t DEFAULT_RESOURCE_ID =
      std::numeric_limits<std::size_t>::max();
  resource_if() = delete;
  resource_if(std::size_t id) : _id(id) {}
  bool has_valid_id() const { return _id != DEFAULT_RESOURCE_ID; }
  virtual ~resource_if() = default;

private:
  std::size_t _id{DEFAULT_RESOURCE_ID};
};

} // namespace truk::core