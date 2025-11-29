#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../render_backend.h"
#include <afterhours/ah.h>
#include <algorithm>

namespace {
inline raylib::Color get_health_color(short health, short max_health) {
  if (max_health <= 1) {
    return raylib::GRAY;
  }

  float health_ratio = static_cast<float>(health) / static_cast<float>(max_health);

  if (health_ratio > 0.75f) {
    return raylib::Color{100, 180, 100, 255};
  } else if (health_ratio > 0.5f) {
    return raylib::Color{200, 200, 120, 255};
  } else if (health_ratio > 0.25f) {
    return raylib::Color{220, 160, 100, 255};
  } else {
    return raylib::Color{200, 100, 100, 255};
  }
}
} // namespace

struct RenderBrick : afterhours::System<BrickGrid> {
  virtual void for_each_with(const afterhours::Entity &,
                             const BrickGrid &brick_grid, float) const override {
    const short max_health = 15;

    for (int y = 0; y < game_constants::GRID_HEIGHT; ++y) {
      for (int x = 0; x < game_constants::GRID_WIDTH; ++x) {
        if (!brick_grid.has_brick(x, y)) {
          continue;
        }

        short health = brick_grid.get_health(x, y);
        raylib::Color color = get_health_color(health, max_health);

        vec2 pos = game_constants::grid_to_world_pos(x, y);
        render_backend::DrawRectangleRec(
            {pos.x, pos.y, game_constants::BRICK_CELL_SIZE,
             game_constants::BRICK_CELL_SIZE},
            color);
      }
    }
  }
};
