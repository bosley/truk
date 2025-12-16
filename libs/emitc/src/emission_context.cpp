#include <truk/emitc/emission_context.hpp>
#include <truk/emitc/emitter.hpp>

namespace truk::emitc {

emission_context_c::emission_context_c(mode_e mode, defer_scope_s *scope,
                                       type_registry_c *types,
                                       variable_registry_c *vars,
                                       builtin_registry_c *builtins,
                                       output_buffer_c *buffer)
    : _mode(mode), _current_scope(scope), _types(types), _variables(vars),
      _builtins(builtins), _buffer(buffer) {}

emission_context_c emission_context_c::with_mode(mode_e new_mode) const {
  return emission_context_c(new_mode, _current_scope, _types, _variables,
                            _builtins, _buffer);
}

emission_context_c emission_context_c::with_scope(defer_scope_s *scope) const {
  return emission_context_c(_mode, scope, _types, _variables, _builtins,
                            _buffer);
}

} // namespace truk::emitc
