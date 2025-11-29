#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

// TODO replace with texture and use shaders
struct RenderFogOfWar : afterhours::System<FogOfWar> {
  virtual void for_each_with(const afterhours::Entity &, const FogOfWar &fog,
                             float) const override {
    for (int grid_y = 0; grid_y < game_constants::GRID_HEIGHT; ++grid_y) {
      for (int grid_x = 0; grid_x < game_constants::GRID_WIDTH; ++grid_x) {
        if (fog.is_revealed(grid_x, grid_y)) {
          continue;
        }

        vec2 world_pos = game_constants::grid_to_world_pos(grid_x, grid_y);
        render_backend::DrawRectangleV(
            world_pos,
            {game_constants::BRICK_CELL_SIZE, game_constants::BRICK_CELL_SIZE},
            raylib::Color{0, 0, 0, 200});
      }
    }
  }
};
