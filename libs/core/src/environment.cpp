#include "truk/core/environment.hpp"

namespace truk::core {

environment_c::~environment_c() {
  _valid->store(false);
  std::lock_guard<std::mutex> lock(_mutex);
}

environment_c::env_mem_handle_ptr environment_c::get_memory_handle() {
  return std::make_unique<environment_memory_handle_c>(*this, _valid);
}

} // namespace truk::core
