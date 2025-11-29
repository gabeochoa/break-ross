#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../render_backend.h"
#include <afterhours/ah.h>
#include <bitset>
#include <vector>

namespace {
inline int grid_index(int grid_x, int grid_y) {
  return grid_y * game_constants::GRID_WIDTH + grid_x;
}

inline int find_rect_width(const BrickGrid &brick_grid,
                           std::bitset<game_constants::GRID_SIZE> &processed,
                           int grid_x, int grid_y) {
  int width = 1;
  while (grid_x + width < game_constants::GRID_WIDTH &&
         brick_grid.has_brick(grid_x + width, grid_y) &&
         !processed[grid_index(grid_x + width, grid_y)]) {
    width++;
  }
  return width;
}

inline int find_rect_height(const BrickGrid &brick_grid,
                            std::bitset<game_constants::GRID_SIZE> &processed,
                            int grid_x, int grid_y, int rect_width) {
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

inline void
mark_rect_as_processed(std::bitset<game_constants::GRID_SIZE> &processed,
                       int grid_x, int grid_y, int rect_width,
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

inline void rebuild_brick_rects_cache(BrickGrid &brick_grid) {
  brick_grid.cached_rects.clear();
  std::bitset<game_constants::GRID_SIZE> processed;

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

      MergedBrickRect rect;
      rect.grid_x = grid_x;
      rect.grid_y = grid_y;
      rect.width = rect_width;
      rect.height = rect_height;
      brick_grid.cached_rects.push_back(rect);
    }
  }
  brick_grid.rects_dirty = false;
}

struct RenderBrick : afterhours::System<BrickGrid> {
  virtual void for_each_with(const afterhours::Entity &,
                             const BrickGrid &brick_grid,
                             float) const override {
    BrickGrid &non_const_grid = const_cast<BrickGrid &>(brick_grid);
    if (non_const_grid.rects_dirty) {
      rebuild_brick_rects_cache(non_const_grid);
    }

    for (const MergedBrickRect &rect : brick_grid.cached_rects) {
      render_merged_rect(rect.grid_x, rect.grid_y, rect.width, rect.height);
    }
  }
};
