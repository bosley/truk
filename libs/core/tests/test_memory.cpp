#include "truk/core/memory.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

class test_item : public truk::core::memory_c::storeable_if {
    int value;
public:
    test_item(int v) : value(v) {}
    int get_value() const { return value; }
    storeable_if* clone() override { return new test_item(value); }
};

TEST_GROUP(MemoryTests) {
    truk::core::memory_c* mem;
    
    void setup() override {
        mem = new truk::core::memory_c();
    }

    void teardown() override {
        delete mem;
    }
};

TEST(MemoryTests, CanConstruct) {
    truk::core::memory_c memory;
}

TEST(MemoryTests, SetAndGetItem) {
    auto item = std::make_unique<test_item>(42);
    mem->set("test_key", std::move(item));
    
    auto* retrieved = mem->get("test_key");
    CHECK_TRUE(retrieved != nullptr);
    
    auto* typed = dynamic_cast<test_item*>(retrieved);
    CHECK_TRUE(typed != nullptr);
    CHECK_EQUAL(42, typed->get_value());
}

TEST(MemoryTests, SetTakesOwnership) {
    auto item = std::make_unique<test_item>(100);
    mem->set("key", std::move(item));
    
    CHECK_TRUE(item == nullptr);
}

TEST(MemoryTests, IsSetReturnsTrueForExistingKey) {
    auto item = std::make_unique<test_item>(1);
    mem->set("exists", std::move(item));
    
    CHECK_TRUE(mem->is_set("exists"));
}

TEST(MemoryTests, IsSetReturnsFalseForNonExistentKey) {
    CHECK_FALSE(mem->is_set("does_not_exist"));
}

TEST(MemoryTests, GetReturnsNullForNonExistentKey) {
    auto* result = mem->get("missing");
    CHECK_TRUE(result == nullptr);
}

TEST(MemoryTests, OverwriteExistingKey) {
    auto item1 = std::make_unique<test_item>(10);
    mem->set("key", std::move(item1));
    
    auto item2 = std::make_unique<test_item>(20);
    mem->set("key", std::move(item2));
    
    auto* retrieved = mem->get("key");
    auto* typed = dynamic_cast<test_item*>(retrieved);
    CHECK_EQUAL(20, typed->get_value());
}

TEST(MemoryTests, DropRemovesItem) {
    auto item = std::make_unique<test_item>(5);
    mem->set("to_drop", std::move(item));
    
    CHECK_TRUE(mem->is_set("to_drop"));
    mem->drop("to_drop");
    CHECK_FALSE(mem->is_set("to_drop"));
}

TEST(MemoryTests, PushContextCreatesNewScope) {
    auto item = std::make_unique<test_item>(1);
    mem->set("root_key", std::move(item));
    
    mem->push_ctx();
    
    CHECK_FALSE(mem->is_set("root_key"));
}

TEST(MemoryTests, PopContextReturnsToParent) {
    auto item1 = std::make_unique<test_item>(1);
    mem->set("root_key", std::move(item1));
    
    mem->push_ctx();
    auto item2 = std::make_unique<test_item>(2);
    mem->set("child_key", std::move(item2));
    
    mem->pop_ctx();
    
    CHECK_TRUE(mem->is_set("root_key"));
    CHECK_FALSE(mem->is_set("child_key"));
}

TEST(MemoryTests, ChildContextDoesNotAffectParent) {
    auto item1 = std::make_unique<test_item>(10);
    mem->set("parent_key", std::move(item1));
    
    mem->push_ctx();
    auto item2 = std::make_unique<test_item>(20);
    mem->set("child_key", std::move(item2));
    
    mem->pop_ctx();
    
    auto* retrieved = mem->get("parent_key");
    auto* typed = dynamic_cast<test_item*>(retrieved);
    CHECK_EQUAL(10, typed->get_value());
}

TEST(MemoryTests, GetWithParentContextSearchesUpChain) {
    auto item = std::make_unique<test_item>(99);
    mem->set("parent_key", std::move(item));
    
    mem->push_ctx();
    
    auto* retrieved = mem->get("parent_key", true);
    CHECK_TRUE(retrieved != nullptr);
    
    auto* typed = dynamic_cast<test_item*>(retrieved);
    CHECK_EQUAL(99, typed->get_value());
}

TEST(MemoryTests, GetWithoutParentContextOnlySearchesCurrent) {
    auto item = std::make_unique<test_item>(50);
    mem->set("parent_key", std::move(item));
    
    mem->push_ctx();
    
    auto* retrieved = mem->get("parent_key", false);
    CHECK_TRUE(retrieved == nullptr);
}

TEST(MemoryTests, DeferHoistMovesItemToParent) {
    mem->push_ctx();
    
    auto item = std::make_unique<test_item>(777);
    mem->set("hoist_key", std::move(item));
    mem->defer_hoist("hoist_key");
    
    mem->pop_ctx();
    
    CHECK_TRUE(mem->is_set("hoist_key"));
    auto* retrieved = mem->get("hoist_key");
    auto* typed = dynamic_cast<test_item*>(retrieved);
    CHECK_EQUAL(777, typed->get_value());
}

TEST(MemoryTests, HoistNonExistentKeyDoesNotCrash) {
    mem->push_ctx();
    mem->defer_hoist("non_existent");
    mem->pop_ctx();
}

TEST(MemoryTests, MultipleHoistsInSameContext) {
    mem->push_ctx();
    
    auto item1 = std::make_unique<test_item>(1);
    auto item2 = std::make_unique<test_item>(2);
    auto item3 = std::make_unique<test_item>(3);
    
    mem->set("key1", std::move(item1));
    mem->set("key2", std::move(item2));
    mem->set("key3", std::move(item3));
    
    mem->defer_hoist("key1");
    mem->defer_hoist("key2");
    mem->defer_hoist("key3");
    
    mem->pop_ctx();
    
    CHECK_TRUE(mem->is_set("key1"));
    CHECK_TRUE(mem->is_set("key2"));
    CHECK_TRUE(mem->is_set("key3"));
}

TEST(MemoryTests, PopContextOnRootIsNoOp) {
    auto item = std::make_unique<test_item>(123);
    mem->set("root_key", std::move(item));
    
    mem->pop_ctx();
    
    CHECK_TRUE(mem->is_set("root_key"));
}

TEST(MemoryTests, NestedContexts) {
    auto item1 = std::make_unique<test_item>(1);
    mem->set("level0", std::move(item1));
    
    mem->push_ctx();
    auto item2 = std::make_unique<test_item>(2);
    mem->set("level1", std::move(item2));
    
    mem->push_ctx();
    auto item3 = std::make_unique<test_item>(3);
    mem->set("level2", std::move(item3));
    
    CHECK_TRUE(mem->is_set("level2"));
    CHECK_FALSE(mem->is_set("level1"));
    CHECK_FALSE(mem->is_set("level0"));
    
    auto* retrieved = mem->get("level0", true);
    CHECK_TRUE(retrieved != nullptr);
    
    mem->pop_ctx();
    CHECK_TRUE(mem->is_set("level1"));
    
    mem->pop_ctx();
    CHECK_TRUE(mem->is_set("level0"));
}

TEST(MemoryTests, HoistAcrossMultipleLevels) {
    mem->push_ctx();
    mem->push_ctx();
    
    auto item = std::make_unique<test_item>(999);
    mem->set("deep_key", std::move(item));
    mem->defer_hoist("deep_key");
    
    mem->pop_ctx();
    
    CHECK_TRUE(mem->is_set("deep_key"));
    mem->defer_hoist("deep_key");
    
    mem->pop_ctx();
    
    CHECK_TRUE(mem->is_set("deep_key"));
    auto* retrieved = mem->get("deep_key");
    auto* typed = dynamic_cast<test_item*>(retrieved);
    CHECK_EQUAL(999, typed->get_value());
}

TEST(MemoryTests, IsSetOnlyChecksCurrentContext) {
    auto item = std::make_unique<test_item>(44);
    mem->set("parent_key", std::move(item));
    
    mem->push_ctx();
    
    CHECK_FALSE(mem->is_set("parent_key"));
}

TEST(MemoryTests, DestructorCleansUpNestedContexts) {
    auto* temp_mem = new truk::core::memory_c();
    
    temp_mem->push_ctx();
    auto item1 = std::make_unique<test_item>(1);
    temp_mem->set("key1", std::move(item1));
    
    temp_mem->push_ctx();
    auto item2 = std::make_unique<test_item>(2);
    temp_mem->set("key2", std::move(item2));
    
    delete temp_mem;
}

int main(int argc, char** argv) {
    return CommandLineTestRunner::RunAllTests(argc, argv);
}
