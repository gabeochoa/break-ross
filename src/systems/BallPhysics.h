#pragma once

#include "../components.h"
#include "../eq.h"
#include "../render_backend.h"
#include "../settings.h"
#include <afterhours/ah.h>
#include <algorithm>

struct BallPhysics
    : afterhours::System<Transform,
                         afterhours::tags::All<ColliderTag::Circle>> {
  float screen_width = 0.0f;
  float screen_height = 0.0f;

  virtual void once(float) override {
    screen_width = static_cast<float>(Settings::get().get_screen_width());
    screen_height = static_cast<float>(Settings::get().get_screen_height());
  }

  virtual void for_each_with(afterhours::Entity &, Transform &transform,
                             float dt) override {

    transform.position.x += transform.velocity.x * dt;
    transform.position.y += transform.velocity.y * dt;

    float radius = transform.size.x / 2.0f;

    float old_x = transform.position.x;
    float old_y = transform.position.y;

    transform.position.x =
        std::max(radius, std::min(screen_width - radius, transform.position.x));
    transform.position.y = std::max(
        radius, std::min(screen_height - radius, transform.position.y));

    if (transform.position.x != old_x) {
      transform.velocity.x = -transform.velocity.x;
    }
    if (transform.position.y != old_y) {
      transform.velocity.y = -transform.velocity.y;
    }
  }
};
