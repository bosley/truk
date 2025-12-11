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

//! \brief The context overflow error is one that is hit when the given
//!        context size of a memory_c<ContextSize> is maxed-out
class context_overflow_error : public std::exception {
public:
  const char *what() const noexcept override {
    return "Maximum context depth exceeded";
  }
};

//! \brief The memory object allows us to manage memory via a k/v store with
//! explicit context
//!        semantics that include "push", "pop", and "hoist". Used to manage any
//!        object that implements the storeable_if interface in a
//!        context-sensitive stack-based manner.
//!
//!        Operations:
//!        - push_ctx: Add a new context layer above the current context.
//!        - pop_ctx: Remove the current context, discarding local keys but
//!        optionally hoisting selected items upward.
//!        - set: Set a value in the current context.
//!        - is_set: Query if a key is present in the current context.
//!        - get: Retrieve a pointer to a value by key. Optionally, check parent
//!        contexts.
//!        - drop: Remove a value from the current context.
//!        - defer_hoist: Mark a key to be hoisted when the context is popped.
template <std::size_t ContextCount = DEFAULT_CONTEXT_COUNT> class memory_c {
public:
  static std::unique_ptr<memory_c<ContextCount>> make_new() {
    return std::make_unique<memory_c<ContextCount>>();
  }

  //! \brief Interface for objects that can be managed by the memory object.
  class storeable_if {
  public:
    virtual ~storeable_if() = default;
    virtual storeable_if *clone() = 0;
  };
  using stored_item_ptr = std::unique_ptr<storeable_if>;

  //! \brief Construct the memory stack with a root context.
  memory_c() : _current(&_root), _ctx_count(1) { _root.parent = nullptr; }

  //! \brief Destructor - pops all stacked contexts (guarantees one root context
  //! always remains).
  ~memory_c() {
    while (_current != &_root) {
      pop_ctx();
    }
  }

  //! \brief Push a new context onto the memory stack.
  //!        Throws context_overflow_error if the max context depth is exceeded.
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

  //! \brief Pop the current context from the stack.
  //!        Hoists any keys scheduled for "defer_hoist" up to the parent
  //!        context. If already at the root context, this is a no-op.
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

  //! \brief Set a value for the specified key in the current context.
  //! \param key The key to assign.
  //! \param item The storeable object to associate with the key.
  void set(const std::string &key, stored_item_ptr item) {
    _current->_scope[key] = std::move(item);
  }

  //! \brief Query if a key is set in the current context.
  //! \param key The key to check.
  //! \return true if present in current context; false otherwise.
  bool is_set(const std::string &key) const {
    return _current->_scope.find(key) != _current->_scope.end();
  }

  //! \brief Get a pointer to the value for a key, optionally searching parent
  //! contexts.
  //! \param key The key to query.
  //! \param use_parent_ctx If true, walk up the parent contexts for the key.
  //! \return Pointer to storeable_if or nullptr if not found.
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

  //! \brief Remove the key and its value from the current context.
  //! \param key The key to erase.
  void drop(const std::string &key) { _current->_scope.erase(key); }

  //! \brief Schedule the given key to be hoisted (moved) up to the parent
  //! context when the stack is popped.
  //! \param key The key to defer-hoist.
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
