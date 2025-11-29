#pragma once

#include "test_app.h"
#include <functional>
#include <map>
#include <string>

struct TestRegistry {
  static TestRegistry &get() {
    static TestRegistry instance;
    return instance;
  }

  void register_test(const std::string &name,
                     std::function<TestApp()> test_func) {
    tests[name] = test_func;
  }

  std::map<std::string, std::function<TestApp()>> tests;
};

#define TEST(name)                                                             \
  static TestApp test_##name();                                                \
  static struct TestRegistrar_##name {                                         \
    TestRegistrar_##name() {                                                   \
      TestRegistry::get().register_test(#name, test_##name);                   \
    }                                                                          \
  } test_registrar_##name;                                                     \
  TestApp test_##name()
