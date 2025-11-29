#pragma once

#include <optional>
#include <queue>

namespace test_input {
struct KeyPress {
  int key;
  bool is_char = false;
  char char_value = 0;
};

extern std::queue<KeyPress> input_queue;
extern bool test_mode;
extern bool slow_test_mode;

void push_key(int key);
void push_char(char c);
void clear_queue();
void reset_frame();

bool is_key_pressed(int key);
int get_char_pressed();

extern bool key_consumed_this_frame;
extern bool char_consumed_this_frame;
} // namespace test_input
