#pragma once

#include <array>
#include <exception>
#include <memory>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace truk::core {

class context_overflow_error : public std::exception {
public:
  const char *what() const noexcept override {
    return "Maximum context depth exceeded";
  }
};

class memory_c {
public:

  // TODO: Make dynamic (later)
  static constexpr std::size_t PRE_ALLOCATED_CONTEXT_COUNT = 256;

  class storeable_if {
  public:
    virtual ~storeable_if() = default;
    virtual storeable_if *clone() = 0;
  };
  using stored_item_ptr = std::unique_ptr<storeable_if>;

  memory_c();
  ~memory_c();

  void push_ctx();
  void pop_ctx();

  void set(const std::string &key, stored_item_ptr item);
  bool is_set(const std::string &key) const;
  storeable_if *get(const std::string &key, bool use_parent_ctx = false);
  void drop(const std::string &key);
  void defer_hoist(const std::string &key);

private:
  struct context_s {
    context_s *parent{nullptr};
    std::unordered_map<std::string, stored_item_ptr> _scope;
    std::queue<std::string> _pending_hoist;
  };

  context_s _root;
  context_s *_current;
  std::array<std::aligned_storage_t<sizeof(context_s), alignof(context_s)>,
             PRE_ALLOCATED_CONTEXT_COUNT>
      _ctx_storage;
  std::size_t _ctx_count;
};

using memory_ptr = std::unique_ptr<memory_c>;

} // namespace truk::core
