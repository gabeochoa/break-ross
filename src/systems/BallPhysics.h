#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include <afterhours/ah.h>
#include <algorithm>
#include <cmath>

struct BallPhysics
    : afterhours::System<Transform,
                         afterhours::tags::All<ColliderTag::Circle>> {
  virtual void once(float) override {}

  virtual void for_each_with(afterhours::Entity &, Transform &transform,
                             float dt) override {
    transform.position.x += transform.velocity.x * dt;
    transform.position.y += transform.velocity.y * dt;

    float radius = transform.size.x / 2.0f;

    bool hit_left = transform.position.x < radius;
    bool hit_right =
        transform.position.x > game_constants::WORLD_WIDTH - radius;
    bool hit_top = transform.position.y < radius;
    bool hit_bottom =
        transform.position.y > game_constants::WORLD_HEIGHT - radius;

    transform.position.x =
        std::max(radius, std::min(game_constants::WORLD_WIDTH - radius,
                                  transform.position.x));
    transform.position.y =
        std::max(radius, std::min(game_constants::WORLD_HEIGHT - radius,
                                  transform.position.y));

    vec2 normal = {0.0f, 0.0f};
    if (hit_left) {
      normal.x = 1.0f;
    } else if (hit_right) {
      normal.x = -1.0f;
    }
    if (hit_top) {
      normal.y = 1.0f;
    } else if (hit_bottom) {
      normal.y = -1.0f;
    }

    float normal_length = std::sqrt(normal.x * normal.x + normal.y * normal.y);
    if (normal_length > 0.0f) {
      normal.x /= normal_length;
      normal.y /= normal_length;

      float dot_product =
          transform.velocity.x * normal.x + transform.velocity.y * normal.y;
      transform.velocity.x -= 2.0f * dot_product * normal.x;
      transform.velocity.y -= 2.0f * dot_product * normal.y;
    }
  }
};
