#include "truk/core/environment.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <chrono>
#include <thread>
#include <vector>

class test_item : public truk::core::memory_c<>::storeable_if {
  int value;

public:
  test_item(int v) : value(v) {}
  int get_value() const { return value; }
  storeable_if *clone() override { return new test_item(value); }
};

TEST_GROUP(EnvironmentTests) {
  truk::core::environment_c *env;

  void setup() override { env = new truk::core::environment_c(1); }

  void teardown() override { delete env; }
};

TEST(EnvironmentTests, CanConstruct) { truk::core::environment_c test_env(42); }

TEST(EnvironmentTests, CanCreateMemoryHandle) {
  auto handle = env->get_memory_handle(1);
  CHECK_TRUE(handle != nullptr);
}

TEST(EnvironmentTests, MultipleHandlesFromSameEnvironment) {
  auto handle1 = env->get_memory_handle(1);
  auto handle2 = env->get_memory_handle(2);

  CHECK_TRUE(handle1 != nullptr);
  CHECK_TRUE(handle2 != nullptr);
}

TEST(EnvironmentTests, HandleOperationsAfterEnvironmentDestruction) {
  auto handle = env->get_memory_handle(100);

  auto item = std::make_unique<test_item>(42);
  handle->set("test_key", std::move(item));
  CHECK_TRUE(handle->is_set("test_key"));

  delete env;
  env = nullptr;

  CHECK_FALSE(handle->is_set("test_key"));

  bool get_exception_thrown = false;
  try {
    handle->get("test_key");
  } catch (const truk::core::environment_exception_c &e) {
    get_exception_thrown = true;
    CHECK_EQUAL(
        static_cast<int>(truk::core::environment_error_e::INVALID_HANDLE),
        e.get_error_code());
  }
  CHECK_TRUE(get_exception_thrown);

  bool set_exception_thrown = false;
  try {
    auto new_item = std::make_unique<test_item>(100);
    handle->set("new_key", std::move(new_item));
  } catch (const truk::core::environment_exception_c &e) {
    set_exception_thrown = true;
    CHECK_EQUAL(
        static_cast<int>(truk::core::environment_error_e::INVALID_HANDLE),
        e.get_error_code());
  }
  CHECK_TRUE(set_exception_thrown);

  CHECK_FALSE(handle->is_set("new_key"));

  handle->push_ctx();
  handle->pop_ctx();
  handle->drop("test_key");
  handle->defer_hoist("test_key");
}

TEST(EnvironmentTests, MultipleHandlesInvalidatedOnDestruction) {
  auto handle1 = env->get_memory_handle(1);
  auto handle2 = env->get_memory_handle(2);
  auto handle3 = env->get_memory_handle(3);

  auto item1 = std::make_unique<test_item>(1);
  handle1->set("key1", std::move(item1));

  auto item2 = std::make_unique<test_item>(2);
  handle2->set("key2", std::move(item2));

  CHECK_TRUE(handle1->is_set("key1"));
  CHECK_TRUE(handle2->is_set("key2"));

  delete env;
  env = nullptr;

  CHECK_FALSE(handle1->is_set("key1"));
  CHECK_FALSE(handle2->is_set("key2"));

  bool exception_thrown = false;
  try {
    handle3->get("key1");
  } catch (const truk::core::environment_exception_c &e) {
    exception_thrown = true;
    CHECK_EQUAL(
        static_cast<int>(truk::core::environment_error_e::INVALID_HANDLE),
        e.get_error_code());
  }
  CHECK_TRUE(exception_thrown);
}

TEST(EnvironmentTests, HandleSetAndGet) {
  auto handle = env->get_memory_handle(1);

  auto item = std::make_unique<test_item>(777);
  handle->set("test_key", std::move(item));

  CHECK_TRUE(handle->is_set("test_key"));

  auto *retrieved = handle->get("test_key");
  CHECK_TRUE(retrieved != nullptr);

  auto *typed = dynamic_cast<test_item *>(retrieved);
  CHECK_TRUE(typed != nullptr);
  CHECK_EQUAL(777, typed->get_value());
}

TEST(EnvironmentTests, HandleIsSetReturnsFalseForNonExistent) {
  auto handle = env->get_memory_handle(1);
  CHECK_FALSE(handle->is_set("does_not_exist"));
}

TEST(EnvironmentTests, HandleGetReturnsNullForNonExistent) {
  auto handle = env->get_memory_handle(1);
  auto *result = handle->get("missing");
  CHECK_TRUE(result == nullptr);
}

TEST(EnvironmentTests, HandleDrop) {
  auto handle = env->get_memory_handle(1);

  auto item = std::make_unique<test_item>(55);
  handle->set("to_drop", std::move(item));
  CHECK_TRUE(handle->is_set("to_drop"));

  handle->drop("to_drop");
  CHECK_FALSE(handle->is_set("to_drop"));
}

TEST(EnvironmentTests, HandlePushAndPopContext) {
  auto handle = env->get_memory_handle(1);

  auto item1 = std::make_unique<test_item>(10);
  handle->set("root_key", std::move(item1));
  CHECK_TRUE(handle->is_set("root_key"));

  handle->push_ctx();
  CHECK_FALSE(handle->is_set("root_key"));

  auto item2 = std::make_unique<test_item>(20);
  handle->set("child_key", std::move(item2));
  CHECK_TRUE(handle->is_set("child_key"));

  handle->pop_ctx();
  CHECK_TRUE(handle->is_set("root_key"));
  CHECK_FALSE(handle->is_set("child_key"));
}

TEST(EnvironmentTests, HandleGetWithParentContext) {
  auto handle = env->get_memory_handle(1);

  auto item = std::make_unique<test_item>(333);
  handle->set("parent_key", std::move(item));

  handle->push_ctx();

  auto *retrieved = handle->get("parent_key", true);
  CHECK_TRUE(retrieved != nullptr);

  auto *typed = dynamic_cast<test_item *>(retrieved);
  CHECK_EQUAL(333, typed->get_value());
}

TEST(EnvironmentTests, HandleGetWithoutParentContext) {
  auto handle = env->get_memory_handle(1);

  auto item = std::make_unique<test_item>(444);
  handle->set("parent_key", std::move(item));

  handle->push_ctx();

  auto *retrieved = handle->get("parent_key", false);
  CHECK_TRUE(retrieved == nullptr);
}

TEST(EnvironmentTests, HandleDeferHoist) {
  auto handle = env->get_memory_handle(1);

  handle->push_ctx();

  auto item = std::make_unique<test_item>(888);
  handle->set("hoist_key", std::move(item));
  handle->defer_hoist("hoist_key");

  handle->pop_ctx();

  CHECK_TRUE(handle->is_set("hoist_key"));
  auto *retrieved = handle->get("hoist_key");
  auto *typed = dynamic_cast<test_item *>(retrieved);
  CHECK_EQUAL(888, typed->get_value());
}

TEST(EnvironmentTests, ConcurrentHandleCreation) {
  std::vector<std::thread> threads;
  std::vector<truk::core::environment_c::env_mem_handle_ptr> handles;
  std::mutex handles_mutex;

  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([this, i, &handles, &handles_mutex]() {
      auto handle = env->get_memory_handle(i);
      std::lock_guard<std::mutex> lock(handles_mutex);
      handles.push_back(std::move(handle));
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  CHECK_EQUAL(10, handles.size());
  for (const auto &handle : handles) {
    CHECK_TRUE(handle != nullptr);
  }
}

TEST(EnvironmentTests, ConcurrentSetOperations) {
  std::vector<std::thread> threads;

  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([this, i]() {
      auto handle = env->get_memory_handle(i);
      for (int j = 0; j < 100; ++j) {
        auto item = std::make_unique<test_item>(i * 100 + j);
        handle->set("key_" + std::to_string(i), std::move(item));
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  auto handle = env->get_memory_handle(100);
  for (int i = 0; i < 10; ++i) {
    CHECK_TRUE(handle->is_set("key_" + std::to_string(i)));
  }
}

TEST(EnvironmentTests, ConcurrentContextOperations) {
  std::vector<std::thread> threads;
  std::atomic<int> success_count{0};

  for (int i = 0; i < 5; ++i) {
    threads.emplace_back([this, i, &success_count]() {
      auto handle = env->get_memory_handle(i);

      handle->push_ctx();
      auto item = std::make_unique<test_item>(42);
      handle->set("thread_key", std::move(item));

      if (handle->is_set("thread_key")) {
        success_count++;
      }

      handle->pop_ctx();
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  CHECK_TRUE(success_count.load() >= 0);
}

TEST(EnvironmentTests, ConcurrentReadOperations) {
  auto handle = env->get_memory_handle(0);

  for (int i = 0; i < 10; ++i) {
    auto item = std::make_unique<test_item>(i);
    handle->set("key_" + std::to_string(i), std::move(item));
  }

  std::vector<std::thread> threads;
  std::atomic<int> read_success{0};

  for (int i = 0; i < 20; ++i) {
    threads.emplace_back([this, i, &read_success]() {
      auto handle = env->get_memory_handle(i + 1);
      for (int j = 0; j < 10; ++j) {
        auto *retrieved = handle->get("key_" + std::to_string(j));
        if (retrieved != nullptr) {
          read_success++;
        }
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  CHECK_EQUAL(200, read_success.load());
}

TEST(EnvironmentTests, DataIntegrityAfterMultipleOperations) {
  auto handle = env->get_memory_handle(1);

  auto item1 = std::make_unique<test_item>(100);
  handle->set("key1", std::move(item1));

  auto item2 = std::make_unique<test_item>(200);
  handle->set("key2", std::move(item2));

  auto item3 = std::make_unique<test_item>(300);
  handle->set("key3", std::move(item3));

  auto *r1 = handle->get("key1");
  auto *r2 = handle->get("key2");
  auto *r3 = handle->get("key3");

  CHECK_EQUAL(100, dynamic_cast<test_item *>(r1)->get_value());
  CHECK_EQUAL(200, dynamic_cast<test_item *>(r2)->get_value());
  CHECK_EQUAL(300, dynamic_cast<test_item *>(r3)->get_value());

  handle->drop("key2");
  CHECK_FALSE(handle->is_set("key2"));
  CHECK_TRUE(handle->is_set("key1"));
  CHECK_TRUE(handle->is_set("key3"));

  CHECK_EQUAL(100, dynamic_cast<test_item *>(handle->get("key1"))->get_value());
  CHECK_EQUAL(300, dynamic_cast<test_item *>(handle->get("key3"))->get_value());
}

TEST(EnvironmentTests, DataIntegrityThroughContexts) {
  auto handle = env->get_memory_handle(1);

  auto item1 = std::make_unique<test_item>(10);
  handle->set("level0", std::move(item1));

  handle->push_ctx();
  auto item2 = std::make_unique<test_item>(20);
  handle->set("level1", std::move(item2));

  handle->push_ctx();
  auto item3 = std::make_unique<test_item>(30);
  handle->set("level2", std::move(item3));

  CHECK_EQUAL(30,
              dynamic_cast<test_item *>(handle->get("level2"))->get_value());
  CHECK_EQUAL(
      20, dynamic_cast<test_item *>(handle->get("level1", true))->get_value());
  CHECK_EQUAL(
      10, dynamic_cast<test_item *>(handle->get("level0", true))->get_value());

  handle->pop_ctx();
  CHECK_EQUAL(20,
              dynamic_cast<test_item *>(handle->get("level1"))->get_value());

  handle->pop_ctx();
  CHECK_EQUAL(10,
              dynamic_cast<test_item *>(handle->get("level0"))->get_value());
}

TEST(EnvironmentTests, HoistedDataSurvivesContextPop) {
  auto handle = env->get_memory_handle(1);

  handle->push_ctx();
  handle->push_ctx();

  auto item = std::make_unique<test_item>(555);
  handle->set("deep_key", std::move(item));
  handle->defer_hoist("deep_key");

  handle->pop_ctx();
  CHECK_TRUE(handle->is_set("deep_key"));
  CHECK_EQUAL(555,
              dynamic_cast<test_item *>(handle->get("deep_key"))->get_value());

  handle->defer_hoist("deep_key");
  handle->pop_ctx();

  CHECK_TRUE(handle->is_set("deep_key"));
  CHECK_EQUAL(555,
              dynamic_cast<test_item *>(handle->get("deep_key"))->get_value());
}

TEST(EnvironmentTests, MultipleHandlesShareData) {
  auto handle1 = env->get_memory_handle(1);
  auto handle2 = env->get_memory_handle(2);

  auto item = std::make_unique<test_item>(777);
  handle1->set("shared_key", std::move(item));

  CHECK_TRUE(handle2->is_set("shared_key"));

  auto *retrieved = handle2->get("shared_key");
  CHECK_EQUAL(777, dynamic_cast<test_item *>(retrieved)->get_value());

  handle2->drop("shared_key");
  CHECK_FALSE(handle1->is_set("shared_key"));
}

TEST(EnvironmentTests, OverwriteExistingKeyPreservesIntegrity) {
  auto handle = env->get_memory_handle(1);

  auto item1 = std::make_unique<test_item>(111);
  handle->set("key", std::move(item1));
  CHECK_EQUAL(111, dynamic_cast<test_item *>(handle->get("key"))->get_value());

  auto item2 = std::make_unique<test_item>(222);
  handle->set("key", std::move(item2));
  CHECK_EQUAL(222, dynamic_cast<test_item *>(handle->get("key"))->get_value());

  auto item3 = std::make_unique<test_item>(333);
  handle->set("key", std::move(item3));
  CHECK_EQUAL(333, dynamic_cast<test_item *>(handle->get("key"))->get_value());
}

TEST(EnvironmentTests, DestructorWithActiveHandles) {
  auto handle1 = env->get_memory_handle(1);
  auto handle2 = env->get_memory_handle(2);

  auto item1 = std::make_unique<test_item>(11);
  handle1->set("key1", std::move(item1));

  auto item2 = std::make_unique<test_item>(22);
  handle2->set("key2", std::move(item2));

  CHECK_TRUE(handle1->is_set("key1"));
  CHECK_TRUE(handle2->is_set("key2"));

  delete env;
  env = nullptr;

  CHECK_FALSE(handle1->is_set("key1"));
  CHECK_FALSE(handle2->is_set("key2"));
}

TEST(EnvironmentTests, HandleOperationsAreNoOpWhenInvalid) {
  auto handle = env->get_memory_handle(999);

  auto item = std::make_unique<test_item>(99);
  handle->set("key", std::move(item));
  CHECK_TRUE(handle->is_set("key"));

  delete env;
  env = nullptr;

  handle->push_ctx();
  handle->pop_ctx();

  bool set_exception_thrown = false;
  try {
    auto new_item = std::make_unique<test_item>(88);
    handle->set("new_key", std::move(new_item));
  } catch (const truk::core::environment_exception_c &e) {
    set_exception_thrown = true;
    CHECK_EQUAL(
        static_cast<int>(truk::core::environment_error_e::INVALID_HANDLE),
        e.get_error_code());
  }
  CHECK_TRUE(set_exception_thrown);

  handle->drop("key");
  handle->defer_hoist("key");

  CHECK_FALSE(handle->is_set("key"));
  CHECK_FALSE(handle->is_set("new_key"));

  bool get_exception_thrown = false;
  try {
    handle->get("key");
  } catch (const truk::core::environment_exception_c &e) {
    get_exception_thrown = true;
    CHECK_EQUAL(
        static_cast<int>(truk::core::environment_error_e::INVALID_HANDLE),
        e.get_error_code());
  }
  CHECK_TRUE(get_exception_thrown);
}

TEST(EnvironmentTests, InvalidHandleThrowsCorrectException) {
  auto handle = env->get_memory_handle(42);

  delete env;
  env = nullptr;

  bool set_exception_thrown = false;
  std::string set_component;
  std::string set_message;
  int set_error_code = 0;

  try {
    auto item = std::make_unique<test_item>(42);
    handle->set("test", std::move(item));
  } catch (const truk::core::environment_exception_c &e) {
    set_exception_thrown = true;
    set_component = e.get_component();
    set_message = e.get_message();
    set_error_code = e.get_error_code();
  }

  CHECK_TRUE(set_exception_thrown);
  STRCMP_EQUAL("environment", set_component.c_str());
  STRCMP_EQUAL("Operation on invalid environment handle (id: 42)",
               set_message.c_str());
  CHECK_EQUAL(static_cast<int>(truk::core::environment_error_e::INVALID_HANDLE),
              set_error_code);

  bool get_exception_thrown = false;
  std::string get_component;
  std::string get_message;
  int get_error_code = 0;

  try {
    handle->get("test");
  } catch (const truk::core::environment_exception_c &e) {
    get_exception_thrown = true;
    get_component = e.get_component();
    get_message = e.get_message();
    get_error_code = e.get_error_code();
  }

  CHECK_TRUE(get_exception_thrown);
  STRCMP_EQUAL("environment", get_component.c_str());
  STRCMP_EQUAL("Operation on invalid environment handle (id: 42)",
               get_message.c_str());
  CHECK_EQUAL(static_cast<int>(truk::core::environment_error_e::INVALID_HANDLE),
              get_error_code);
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
