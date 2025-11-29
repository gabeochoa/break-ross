#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../render_backend.h"
#include <afterhours/ah.h>
#include <vector>

namespace {
inline int grid_index(int grid_x, int grid_y) {
  return grid_y * game_constants::GRID_WIDTH + grid_x;
}

inline int find_rect_width(const BrickGrid &brick_grid,
                           std::vector<bool> &processed, int grid_x,
                           int grid_y) {
  int width = 1;
  while (grid_x + width < game_constants::GRID_WIDTH &&
         brick_grid.has_brick(grid_x + width, grid_y) &&
         !processed[grid_index(grid_x + width, grid_y)]) {
    width++;
  }
  return width;
}

inline int find_rect_height(const BrickGrid &brick_grid,
                            std::vector<bool> &processed, int grid_x,
                            int grid_y, int rect_width) {
  int height = 1;
  while (grid_y + height < game_constants::GRID_HEIGHT) {
    bool can_extend = true;
    for (int x = 0; x < rect_width; ++x) {
      if (!brick_grid.has_brick(grid_x + x, grid_y + height) ||
          processed[grid_index(grid_x + x, grid_y + height)]) {
        can_extend = false;
        break;
      }
    }
    if (!can_extend) {
      break;
    }
    height++;
  }
  return height;
}

inline void mark_rect_as_processed(std::vector<bool> &processed, int grid_x,
                                   int grid_y, int rect_width,
                                   int rect_height) {
  for (int y = 0; y < rect_height; ++y) {
    for (int x = 0; x < rect_width; ++x) {
      processed[grid_index(grid_x + x, grid_y + y)] = true;
    }
  }
}

inline void render_merged_rect(int grid_x, int grid_y, int rect_width,
                               int rect_height) {
  vec2 pos = game_constants::grid_to_world_pos(grid_x, grid_y);
  render_backend::DrawRectangleRec(
      {pos.x, pos.y, rect_width * game_constants::BRICK_CELL_SIZE,
       rect_height * game_constants::BRICK_CELL_SIZE},
      raylib::GRAY);
}
} // namespace

struct RenderBrick : afterhours::System<BrickGrid> {
  virtual void for_each_with(const afterhours::Entity &,
                             const BrickGrid &brick_grid,
                             float) const override {
    std::vector<bool> processed(game_constants::GRID_SIZE, false);

    for (int grid_y = 0; grid_y < game_constants::GRID_HEIGHT; ++grid_y) {
      for (int grid_x = 0; grid_x < game_constants::GRID_WIDTH; ++grid_x) {
        if (processed[grid_index(grid_x, grid_y)] ||
            !brick_grid.has_brick(grid_x, grid_y)) {
          continue;
        }

        int rect_width = find_rect_width(brick_grid, processed, grid_x, grid_y);
        int rect_height =
            find_rect_height(brick_grid, processed, grid_x, grid_y, rect_width);

        mark_rect_as_processed(processed, grid_x, grid_y, rect_width,
                               rect_height);
        render_merged_rect(grid_x, grid_y, rect_width, rect_height);
      }
    }
  }
};
