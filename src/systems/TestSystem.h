#pragma once

#include "../testing/test_app.h"
#include "../testing/test_input.h"
#include <afterhours/ah.h>
#include <chrono>
#include <optional>
#include <thread>

namespace test_app {
extern int frame_counter;
}

struct TestSystem : afterhours::System<> {
  std::optional<TestApp> current_test;
  std::string test_name;
  bool test_complete = false;
  std::string test_error;

  void set_test(const std::string &name, TestApp test) {
    test_name = name;
    current_test = std::move(test);
    test_complete = false;
    test_error.clear();
    test_input::test_mode = true;
    test_input::clear_queue();
    test_app::frame_counter = 0;
  }

  void once(float) override {
    test_input::reset_frame();
    test_app::frame_counter++;

    if (!current_test.has_value()) {
      return;
    }

    if (test_complete) {
      return;
    }

    if (current_test->handle && !current_test->handle.done()) {
      current_test->resume();

      if (test_input::slow_test_mode) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
      }
    }

    if (current_test->is_done()) {
      test_complete = true;
      test_input::test_mode = false;
      std::string error = current_test->get_error();
      if (!error.empty()) {
        test_error = error;
      }
      current_test.reset();
    }
  }

  bool is_complete() const { return test_complete; }
  std::string get_error() const { return test_error; }
  std::string get_test_name() const { return test_name; }
};
