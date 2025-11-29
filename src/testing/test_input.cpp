#include "test_input.h"
#include "../rl.h"

namespace test_input {
std::queue<KeyPress> input_queue;
bool test_mode = false;
bool slow_test_mode = false;

void push_key(int key) {
  KeyPress press;
  press.key = key;
  press.is_char = false;
  input_queue.push(press);
}

void push_char(char c) {
  KeyPress press;
  press.key = 0;
  press.is_char = true;
  press.char_value = c;
  input_queue.push(press);
}

void clear_queue() {
  while (!input_queue.empty()) {
    input_queue.pop();
  }
}

bool key_consumed_this_frame = false;
bool char_consumed_this_frame = false;

bool is_key_pressed(int key) {
  if (!test_mode || input_queue.empty() || key_consumed_this_frame) {
    return raylib::IsKeyPressed(key);
  }

  if (!input_queue.front().is_char && input_queue.front().key == key) {
    input_queue.pop();
    key_consumed_this_frame = true;
    return true;
  }

  return raylib::IsKeyPressed(key);
}

int get_char_pressed() {
  if (!test_mode || input_queue.empty() || char_consumed_this_frame) {
    return raylib::GetCharPressed();
  }

  if (input_queue.front().is_char) {
    char c = input_queue.front().char_value;
    input_queue.pop();
    char_consumed_this_frame = true;
    return static_cast<int>(c);
  }

  return raylib::GetCharPressed();
}

void reset_frame() {
  key_consumed_this_frame = false;
  char_consumed_this_frame = false;
}
} // namespace test_input
