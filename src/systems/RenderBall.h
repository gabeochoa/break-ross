#pragma once

#include "../components.h"
#include "../eq.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

struct RenderBall
    : afterhours::System<Transform,
                         afterhours::tags::All<ColliderTag::Circle>> {
  virtual void for_each_with(const afterhours::Entity &,
                             const Transform &transform, float) const override {

    render_backend::DrawCircleV(
        {transform.position.x + transform.size.x / 2.0f,
         transform.position.y + transform.size.y / 2.0f},
        transform.size.x / 2.0f, raylib::WHITE);
  }
};
