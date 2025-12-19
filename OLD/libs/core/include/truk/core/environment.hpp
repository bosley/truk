#pragma once

#include "exceptions.hpp"
#include "memory.hpp"
#include "resource.hpp"
#include <atomic>
#include <fmt/format.h>
#include <memory>
#include <mutex>

/*
To access the memory of an environment, get a memory
handle off the environment object. If the environment is terminated while
handles still exist, The handle functionality will NOP. I made this to ensure
that we have all memory and data accounted for no matter what the use-case is as
memory.hpp stores a storeable_if. This means That ANY object that implements
that interface can be fully managed via a memory system within and environment

We offer the push/pop/hoist pattern so if there are nested environemnts that
spawn objects, you can "hoist" their lifetime to their parent context explicitly
*/

namespace truk::core {

enum class environment_error_e : int { INVALID_HANDLE = 1 };

//! \brief The envrionment_c object controls the access to a memory object.
//!        We do this to offer multiple r/w handles to the same memory object
//!        while also handling concurrency issues.
//! \note  This is-a resource that the core can manage
class environment_c : public resource_if {
public:
  environment_c() = delete;

  environment_c(const environment_c &) = delete;
  environment_c &operator=(const environment_c &) = delete;

  environment_c(environment_c &&) = delete;
  environment_c &operator=(environment_c &&) = delete;

  ~environment_c();

  //! \brief We are a managed resource so we setup with id and default context
  //! sized memory.
  //! \param id The external id to reference this object by for external
  //! management via the resource interface
  //! \note In the future we may change how we size the prealloced routed memory
  //! from DEFAULT_CONTEXT_COUNT to
  //!       some other blocking or otherwise dynamic means. This _could_ mean
  //!       BIG changes to this interface.
  environment_c(std::size_t id)
      : resource_if(id), _memory(memory_c<DEFAULT_CONTEXT_COUNT>::make_new()),
        _valid(std::make_shared<std::atomic<bool>>(true)) {}

  const char *get_resource_description() override {
    return "environment_c [memory access management]";
  }

  /*
      The handle is a pass-through wrapper around the environment's internal
     memory object that handles validation of pointer-to the environment object
     as-well-as locks the object for concurrency sake.

      We could make a more dynamic/complex locking system but until demand for
     it arises i want to keep it simple. With the current setup we have 1 memory
     object contained by 1 environment with N handles to the environment. The
     handles all have a shared pointer to a validation atomic that ensures if
     the environment_c was for some reason demolished (crash or w/e) that thte
     handles begin to NOP rather than potentially segfault. Once validated they
     then attempt to grab the mutex off of the environment to lock access to the
     memory object. This means that each handle SHARES access and _can_ be a
     congestion point.

      At the core level its hard to say if this will bottleneck anything, but if
     the situation demands lots of r/w handles to the same memory object then we
     should either update theis to be more intelligent about concurrent access
     OR the user (you, bosley) will need to consider if the environment object
     is the right object for the task. It might be beneficial to instead,
     implement a custom object similar-to environment_c that acts as the
     pass-through.

      We _could_ make an abstract pattern factory builder for this but good
     christ i don't want to.

      My intent with environment_c is to KISS. Its just to give some promises to
     the access of the memory contexts

      When we go to load external libs well probably make an env and in the root
     memory object context-out something to hold each loaded library as a
     storeable_if either through RLL or through some custom means
  */
  class environment_memory_handle_c {
  public:
    environment_memory_handle_c() = delete;
    environment_memory_handle_c(environment_c &env,
                                std::shared_ptr<std::atomic<bool>> valid,
                                std::size_t id)
        : _env(env), _valid(valid), _id(id) {}

    void push_ctx() {
      if (!_valid->load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      _env._memory->push_ctx();
    }

    void pop_ctx() {
      if (!_valid->load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      _env._memory->pop_ctx();
    }

    void set(const std::string &key, memory_c<>::stored_item_ptr item) {
      if (!_valid->load()) {
        throw environment_exception_c(
            static_cast<int>(environment_error_e::INVALID_HANDLE),
            fmt::format("Operation on invalid environment handle (id: {})",
                        _id));
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      _env._memory->set(key, std::move(item));
    }

    bool is_set(const std::string &key) const {
      if (!_valid->load()) {
        return false;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      return _env._memory->is_set(key);
    }

    memory_c<>::storeable_if *get(const std::string &key,
                                  bool use_parent_ctx = false) {
      if (!_valid->load()) {
        throw environment_exception_c(
            static_cast<int>(environment_error_e::INVALID_HANDLE),
            fmt::format("Operation on invalid environment handle (id: {})",
                        _id));
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      return _env._memory->get(key, use_parent_ctx);
    }

    void drop(const std::string &key) {
      if (!_valid->load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      _env._memory->drop(key);
    }

    void defer_hoist(const std::string &key) {
      if (!_valid->load()) {
        return;
      }
      std::lock_guard<std::mutex> lock(_env._mutex);
      _env._memory->defer_hoist(key);
    }

  private:
    environment_c &_env;
    std::shared_ptr<std::atomic<bool>> _valid;
    std::size_t _id;
  };

  // there is so much sugar in c++ syntax its hard to swallow sometimes
  using env_mem_handle_ptr = std::unique_ptr<environment_memory_handle_c>;

  env_mem_handle_ptr get_memory_handle(std::size_t id);

private:
  memory_ptr _memory{nullptr};
  std::mutex _mutex;
  std::shared_ptr<std::atomic<bool>> _valid;
};

using env_ptr = std::unique_ptr<environment_c>;

} // namespace truk::core
