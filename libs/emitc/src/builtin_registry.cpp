#include <truk/emitc/builtin_handler.hpp>

namespace truk::emitc {

void builtin_registry_c::register_handler(
    const std::string &name, std::unique_ptr<builtin_handler_if> handler) {
  _handlers[name] = std::move(handler);
}

builtin_handler_if *
builtin_registry_c::get_handler(const std::string &name) const {
  auto it = _handlers.find(name);
  if (it != _handlers.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool builtin_registry_c::is_builtin(const std::string &name) const {
  return _handlers.find(name) != _handlers.end();
}

} // namespace truk::emitc
