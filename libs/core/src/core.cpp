#include "truk/core/core.hpp"
#include "truk/build_info.hpp"
#include <fmt/core.h>

namespace truk::core {

struct core_c::impl {
  bool initialized = false;
  std::string build_hash;

  impl() : build_hash(truk::BUILD_HASH) {}
};

core_c::core_c() : pimpl_(std::make_unique<impl>()) {}

core_c::~core_c() = default;

core_c::core_c(core_c &&) noexcept = default;
core_c &core_c::operator=(core_c &&) noexcept = default;

std::string core_c::get_build_hash() const { return pimpl_->build_hash; }

bool core_c::is_initialized() const { return pimpl_->initialized; }

void core_c::initialize() {
  if (pimpl_->initialized) {
    return;
  }
  pimpl_->initialized = true;
}

void core_c::shutdown() {
  if (!pimpl_->initialized) {
    return;
  }
  pimpl_->initialized = false;
}

} // namespace truk::core
