#pragma once

#include <truk/emitc/builtin_handler.hpp>
#include <truk/emitc/output_buffer.hpp>
#include <truk/emitc/type_registry.hpp>
#include <truk/emitc/variable_registry.hpp>

namespace truk::emitc {

struct defer_scope_s;

class emission_context_c {
public:
  enum class mode_e { DECLARATION, STATEMENT, EXPRESSION };

  emission_context_c(mode_e mode, defer_scope_s *scope, type_registry_c *types,
                     variable_registry_c *vars, builtin_registry_c *builtins,
                     output_buffer_c *buffer);

  mode_e mode() const { return _mode; }
  defer_scope_s *current_scope() const { return _current_scope; }
  type_registry_c *types() const { return _types; }
  variable_registry_c *variables() const { return _variables; }
  builtin_registry_c *builtins() const { return _builtins; }
  output_buffer_c *buffer() const { return _buffer; }

  emission_context_c with_mode(mode_e new_mode) const;
  emission_context_c with_scope(defer_scope_s *scope) const;

private:
  mode_e _mode;
  defer_scope_s *_current_scope;
  type_registry_c *_types;
  variable_registry_c *_variables;
  builtin_registry_c *_builtins;
  output_buffer_c *_buffer;
};

} // namespace truk::emitc
