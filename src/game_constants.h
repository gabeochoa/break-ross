#pragma once

#include "rl.h"

namespace game_constants {
constexpr float BRICK_START_X = 50.0f;
constexpr float BRICK_START_Y = 50.0f;
constexpr float BRICK_SIZE = 30.0f;
constexpr float BRICK_SPACING = 5.0f;
constexpr float BRICK_CELL_SIZE = BRICK_SIZE + BRICK_SPACING;

constexpr int GRID_WIDTH = 100;
constexpr int GRID_HEIGHT = 50;
constexpr int GRID_SIZE = GRID_WIDTH * GRID_HEIGHT;

constexpr float WORLD_WIDTH = BRICK_START_X + (GRID_WIDTH * BRICK_CELL_SIZE);
constexpr float WORLD_HEIGHT = BRICK_START_Y + (GRID_HEIGHT * BRICK_CELL_SIZE);

inline int world_to_grid_x(float world_x) {
  return static_cast<int>((world_x - BRICK_START_X) / BRICK_CELL_SIZE);
}

inline int world_to_grid_y(float world_y) {
  return static_cast<int>((world_y - BRICK_START_Y) / BRICK_CELL_SIZE);
}

inline vec2 grid_to_world_pos(int grid_x, int grid_y) {
  return {BRICK_START_X + grid_x * BRICK_CELL_SIZE,
          BRICK_START_Y + grid_y * BRICK_CELL_SIZE};
}
} // namespace game_constants
