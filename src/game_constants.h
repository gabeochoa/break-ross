#pragma once

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
} // namespace game_constants
