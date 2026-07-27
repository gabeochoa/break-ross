#pragma once

#include "../components.h"
#include "../eq.h"
#include "../palette.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

struct RenderCar
    : afterhours::System<Transform, RoadFollowing,
                         afterhours::tags::All<ColliderTag::Circle>> {
  virtual void for_each_with(const afterhours::Entity &,
                             const Transform &transform,
                             const RoadFollowing &, float) const override {
    // The player's scan cars are always the accent color (rivals will be blue).
    render_backend::DrawCircleV(transform.position, transform.size.x / 2.0f,
                                palette::ACCENT);
  }
};
