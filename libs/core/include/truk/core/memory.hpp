#pragma once

#include <array>
#include <exception>
#include <memory>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace truk::core {

static constexpr std::size_t DEFAULT_CONTEXT_COUNT = 256;

class context_overflow_error : public std::exception {
public:
  const char *what() const noexcept override {
    return "Maximum context depth exceeded";
  }
};

template <std::size_t ContextCount = DEFAULT_CONTEXT_COUNT> class memory_c {
public:
  static std::unique_ptr<memory_c<ContextCount>> make_new() {
    return std::make_unique<memory_c<ContextCount>>();
  }

  class storeable_if {
  public:
    virtual ~storeable_if() = default;
    virtual storeable_if *clone() = 0;
  };
  using stored_item_ptr = std::unique_ptr<storeable_if>;

  memory_c() : _current(&_root), _ctx_count(1) { _root.parent = nullptr; }

  ~memory_c() {
    while (_current != &_root) {
      pop_ctx();
    }
  }

  void push_ctx() {
    if (_ctx_count >= ContextCount) {
      throw context_overflow_error();
    }
    void *storage_ptr = &_ctx_storage[_ctx_count - 1];
    auto *new_ctx = new (storage_ptr) context_s();
    new_ctx->parent = _current;
    _current = new_ctx;
    ++_ctx_count;
  }

  void pop_ctx() {
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

  void set(const std::string &key, stored_item_ptr item) {
    _current->_scope[key] = std::move(item);
  }

  bool is_set(const std::string &key) const {
    return _current->_scope.find(key) != _current->_scope.end();
  }

  storeable_if *get(const std::string &key, bool use_parent_ctx = false) {
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

  void drop(const std::string &key) { _current->_scope.erase(key); }

  void defer_hoist(const std::string &key) {
    _current->_pending_hoist.push(key);
  }

private:
  struct context_s {
    context_s *parent{nullptr};
    std::unordered_map<std::string, stored_item_ptr> _scope;
    std::queue<std::string> _pending_hoist;
  };

  context_s _root;
  context_s *_current;
  std::array<std::aligned_storage_t<sizeof(context_s), alignof(context_s)>,
             ContextCount>
      _ctx_storage;
  std::size_t _ctx_count;
};

using memory_ptr = std::unique_ptr<memory_c<>>;

} // namespace truk::core
