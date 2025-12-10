#include "truk/core/environment.hpp"

namespace truk::core {

environment_c::environment_c(environment_c &&other) noexcept
    : resource_if(std::move(other)), _memory(std::move(other._memory)),
      _complete(other._complete.load()) {}

environment_c &environment_c::operator=(environment_c &&other) noexcept {
  if (this != &other) {
    _complete.store(true);

    resource_if::operator=(std::move(other));
    _memory = std::move(other._memory);
    _complete.store(other._complete.load());
  }
  return *this;
}

environment_c::~environment_c() {
  _complete.store(true);
  std::lock_guard<std::mutex> lock(_mutex);
}

environment_c::env_mem_handle_ptr environment_c::get_memory_handle() {
  return std::make_unique<environment_memory_handle_c>(*this);
}

} // namespace truk::core
