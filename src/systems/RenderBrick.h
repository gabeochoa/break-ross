#pragma once

#include "../components.h"
#include "../eq.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

struct RenderBrick
    : afterhours::System<Transform, HasHealth,
                         afterhours::tags::All<ColliderTag::Rect>> {
  virtual void for_each_with(const afterhours::Entity &,
                             const Transform &transform, const HasHealth &,
                             float) const override {
    render_backend::DrawRectangleRec(
        {transform.position.x, transform.position.y,
         game_constants::BRICK_CELL_SIZE, game_constants::BRICK_CELL_SIZE},
        raylib::GRAY);
  }
};
