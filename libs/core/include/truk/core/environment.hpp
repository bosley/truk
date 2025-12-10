#pragma once

#include "memory.hpp"
#include "resource.hpp"
#include <memory>
#include <mutex>

namespace truk::core {

class environment_c : public resource_if {
public:
  class environment_memory_handle_c {
  public:
    environment_memory_handle_c() = delete;
    environment_memory_handle_c(environment_c &env) : _env(env) {}

    void push_ctx() {
      if (_env._complete.load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      _env._memory->push_ctx();
    }

    void pop_ctx() {
      if (_env._complete.load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      _env._memory->pop_ctx();
    }

    void set(const std::string &key, memory_c<>::stored_item_ptr item) {
      if (_env._complete.load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      _env._memory->set(key, std::move(item));
    }

    bool is_set(const std::string &key) const {
      if (_env._complete.load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      return _env._memory->is_set(key);
    }

    memory_c<>::storeable_if *get(const std::string &key,
                                  bool use_parent_ctx = false) {
      if (_env._complete.load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      return _env._memory->get(key, use_parent_ctx);
    }

    void drop(const std::string &key) {
      if (_env._complete.load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      _env._memory->drop(key);
    }

    void defer_hoist(const std::string &key) {
      if (_env._complete.load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      _env._memory->defer_hoist(key);
    }

  private:
    environment_c &_env;
  };
  using env_mem_handle_ptr = std::unique_ptr<environment_memory_handle_c>;

  environment_c() = delete;

  environment_c(const environment_c &) = delete;
  environment_c &operator=(const environment_c &) = delete;

  environment_c(environment_c &&) noexcept;
  environment_c &operator=(environment_c &&) noexcept;

  environment_c(std::size_t id)
      : resource_if(id), _memory(memory_c<DEFAULT_CONTEXT_COUNT>::make_new()) {}

  env_mem_handle_ptr get_memory_handle();

private:
  memory_ptr _memory{nullptr};
  std::mutex _mutex;
  std::atomic<bool> _complete{false};
};

using env_ptr = std::unique_ptr<environment_c>;

} // namespace truk::core
