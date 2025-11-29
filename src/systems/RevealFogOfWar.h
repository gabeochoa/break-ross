#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../log.h"
#include <afterhours/ah.h>
#include <cmath>

struct RevealFogOfWar
    : afterhours::System<Transform,
                         afterhours::tags::All<ColliderTag::Square>> {
  virtual void for_each_with(afterhours::Entity &, Transform &transform,
                             float) override {
    FogOfWar *fog = afterhours::EntityHelper::get_singleton_cmp<FogOfWar>();
    invariant(fog != nullptr, "FogOfWar singleton component must exist");

    float reveal_radius = fog->reveal_radius;
    float reveal_radius_sq = reveal_radius * reveal_radius;

    int center_grid_x = game_constants::world_to_grid_x(transform.position.x);
    int center_grid_y = game_constants::world_to_grid_y(transform.position.y);

    int radius_in_cells = static_cast<int>(
        std::ceil(reveal_radius / game_constants::BRICK_CELL_SIZE));

    for (int dy = -radius_in_cells; dy <= radius_in_cells; ++dy) {
      for (int dx = -radius_in_cells; dx <= radius_in_cells; ++dx) {
        int grid_x = center_grid_x + dx;
        int grid_y = center_grid_y + dy;

        if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
            grid_y >= game_constants::GRID_HEIGHT) {
          continue;
        }

        if (fog->is_revealed(grid_x, grid_y)) {
          continue;
        }

        vec2 cell_world_pos = game_constants::grid_to_world_pos(grid_x, grid_y);
        vec2 cell_center = {
            cell_world_pos.x + game_constants::BRICK_CELL_SIZE * 0.5f,
            cell_world_pos.y + game_constants::BRICK_CELL_SIZE * 0.5f};

        float dx_world = cell_center.x - transform.position.x;
        float dy_world = cell_center.y - transform.position.y;
        float dist_sq = dx_world * dx_world + dy_world * dy_world;

        if (dist_sq <= reveal_radius_sq) {
          fog->set_revealed(grid_x, grid_y);
        }
      }
    }
  }
};
