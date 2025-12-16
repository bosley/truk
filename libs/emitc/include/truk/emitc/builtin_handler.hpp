#pragma once

#include <language/node.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace truk::emitc {

class emitter_c;

class builtin_handler_if {
public:
  virtual ~builtin_handler_if() = default;

  virtual void emit_call(const truk::language::nodes::call_c &node,
                         emitter_c &emitter) = 0;
};

class builtin_registry_c {
public:
  builtin_registry_c() = default;
  ~builtin_registry_c() = default;

  void register_handler(const std::string &name,
                        std::unique_ptr<builtin_handler_if> handler);
  builtin_handler_if *get_handler(const std::string &name) const;
  bool is_builtin(const std::string &name) const;

private:
  std::unordered_map<std::string, std::unique_ptr<builtin_handler_if>>
      _handlers;
};

void register_builtin_handlers(builtin_registry_c &registry);

} // namespace truk::emitc
