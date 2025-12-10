#include "truk/core/memory.hpp"

namespace truk::core {

memory_c::memory_c() : _current(&_root), _ctx_count(1) {
  _root.parent = nullptr;
}

memory_c::~memory_c() {
  while (_current != &_root) {
    pop_ctx();
  }
}

void memory_c::push_ctx() {
  if (_ctx_count >= PRE_ALLOCATED_CONTEXT_COUNT) {
    throw context_overflow_error();
  }
  void *storage_ptr = &_ctx_storage[_ctx_count - 1];
  auto *new_ctx = new (storage_ptr) context_s();
  new_ctx->parent = _current;
  _current = new_ctx;
  ++_ctx_count;
}

void memory_c::pop_ctx() {
  if (_current == &_root) {
    return;
  }

  while (!_current->_pending_hoist.empty()) {
    const auto &key = _current->_pending_hoist.front();
    auto it = _current->_scope.find(key);
    if (it != _current->_scope.end()) {
      _current->parent->_scope[key] = std::move(it->second);
    }
    _current->_pending_hoist.pop();
  }

  auto *old_ctx = _current;
  _current = _current->parent;
  old_ctx->~context_s();
  --_ctx_count;
}

void memory_c::set(const std::string &key, stored_item_ptr item) {
  _current->_scope[key] = std::move(item);
}

bool memory_c::is_set(const std::string &key) const {
  return _current->_scope.find(key) != _current->_scope.end();
}

memory_c::storeable_if *memory_c::get(const std::string &key,
                                      bool use_parent_ctx) {
  auto *ctx = _current;
  while (ctx != nullptr) {
    auto it = ctx->_scope.find(key);
    if (it != ctx->_scope.end()) {
      return it->second.get();
    }
    if (!use_parent_ctx) {
      break;
    }
    ctx = ctx->parent;
  }
  return nullptr;
}

void memory_c::drop(const std::string &key) { _current->_scope.erase(key); }

void memory_c::defer_hoist(const std::string &key) {
  _current->_pending_hoist.push(key);
}

} // namespace truk::core
