#pragma once

#include "../input_wrapper.h"
#include "../rl.h"
#include <afterhours/ah.h>
#include <afterhours/src/plugins/camera.h>

struct HandleCameraControls : afterhours::System<afterhours::camera::HasCamera> {
  virtual void for_each_with(afterhours::Entity &,
                             afterhours::camera::HasCamera &camera,
                             float dt) override {
    float move_speed = 500.0f * dt;
    float zoom_speed = 2.0f * dt;

    vec2 move_delta{0.0f, 0.0f};

    if (game_input::IsKeyDown(raylib::KEY_W) ||
        game_input::IsKeyDown(raylib::KEY_UP)) {
      move_delta.y -= move_speed;
    }
    if (game_input::IsKeyDown(raylib::KEY_S) ||
        game_input::IsKeyDown(raylib::KEY_DOWN)) {
      move_delta.y += move_speed;
    }
    if (game_input::IsKeyDown(raylib::KEY_A) ||
        game_input::IsKeyDown(raylib::KEY_LEFT)) {
      move_delta.x -= move_speed;
    }
    if (game_input::IsKeyDown(raylib::KEY_D) ||
        game_input::IsKeyDown(raylib::KEY_RIGHT)) {
      move_delta.x += move_speed;
    }

    if (move_delta.x != 0.0f || move_delta.y != 0.0f) {
      vec2 current_pos = camera.camera.target;
      camera.set_position({current_pos.x + move_delta.x,
                           current_pos.y + move_delta.y});
    }

    float current_zoom = camera.camera.zoom;
    if (game_input::IsKeyDown(raylib::KEY_Q)) {
      float new_zoom = current_zoom - zoom_speed;
      if (new_zoom < 0.1f) {
        new_zoom = 0.1f;
      }
      camera.set_zoom(new_zoom);
    }
    if (game_input::IsKeyDown(raylib::KEY_E)) {
      float new_zoom = current_zoom + zoom_speed;
      if (new_zoom > 5.0f) {
        new_zoom = 5.0f;
      }
      camera.set_zoom(new_zoom);
    }
  }
};

