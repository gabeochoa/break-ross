#pragma once

#include "../components.h"
#include "../eq.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

struct RenderSquare
    : afterhours::System<Transform,
                         afterhours::tags::All<ColliderTag::Square>> {
  virtual void for_each_with(const afterhours::Entity &,
                             const Transform &transform, float) const override {
    render_backend::DrawRectangleV(transform.position, transform.size,
                                   raylib::WHITE);
  }
};
