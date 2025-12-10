#include "truk/core/environment.hpp"

namespace truk::core {

environment_c::environment_c(environment_c &&other) noexcept
    : resource_if(std::move(other)), _memory(std::move(other._memory)),
      _valid(std::move(other._valid)) {
  other._valid = std::make_shared<std::atomic<bool>>(false);
}

environment_c &environment_c::operator=(environment_c &&other) noexcept {
  if (this != &other) {
    _valid->store(false);

    resource_if::operator=(std::move(other));
    _memory = std::move(other._memory);
    _valid = std::move(other._valid);
    other._valid = std::make_shared<std::atomic<bool>>(false);
  }
  return *this;
}

environment_c::~environment_c() {
  _valid->store(false);
  std::lock_guard<std::mutex> lock(_mutex);
}

environment_c::env_mem_handle_ptr environment_c::get_memory_handle() {
  return std::make_unique<environment_memory_handle_c>(*this, _valid);
}

} // namespace truk::core
