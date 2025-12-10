#pragma once

#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

namespace truk::core {

class memory_c {
public:
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
};

using memory_ptr = std::unique_ptr<memory_c>;

} // namespace truk::core
