#pragma once

#include "game_constants.h"
#include "log.h"
#include "rl.h"
#include "std_include.h"
#include <afterhours/ah.h>
#include <bitset>
#include <magic_enum/magic_enum.hpp>

struct Transform : afterhours::BaseComponent {
  vec2 position{0.f, 0.f};
  vec2 velocity{0.f, 0.f};
  vec2 size{0.f, 0.f};

  Transform() = default;
  Transform(vec2 pos, vec2 vel, vec2 sz)
      : position(pos), velocity(vel), size(sz) {}
};

enum struct ColliderTag : afterhours::TagId {
  Circle = 0,
  Rect = 1,
};

struct HasHealth : afterhours::BaseComponent {
  int max_amount{0};
  int amount{0};

  HasHealth() = default;
  HasHealth(int max_amount_in)
      : max_amount(max_amount_in), amount(max_amount_in) {}
  HasHealth(int max_amount_in, int amount_in)
      : max_amount(max_amount_in), amount(amount_in) {}
};

struct CanDamage : afterhours::BaseComponent {
  afterhours::EntityID id;
  int amount;

  CanDamage() = default;
  CanDamage(afterhours::EntityID id_in, int amount_in)
      : id(id_in), amount(amount_in) {}
};

struct IsShopManager : afterhours::BaseComponent {
  int ball_cost;
  int ball_damage;
  int pixels_collected;

  IsShopManager() = default;
  IsShopManager(int cost, int damage, int pixels_in)
      : ball_cost(cost), ball_damage(damage), pixels_collected(pixels_in) {}
};

struct RevealedRect {
  float x;
  float y;
  float width;
  float height;
};

struct IsPhotoReveal : afterhours::BaseComponent {
  std::bitset<game_constants::GRID_SIZE> revealed_cells;
  std::vector<RevealedRect> merged_rects;
  float cell_size;

  IsPhotoReveal() = default;
  IsPhotoReveal(float cell_size_in) : cell_size(cell_size_in) {}

  bool is_revealed(int grid_x, int grid_y) const {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return false;
    }
    return revealed_cells[grid_y * game_constants::GRID_WIDTH + grid_x];
  }

  void set_revealed(int grid_x, int grid_y) {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return;
    }
    revealed_cells[grid_y * game_constants::GRID_WIDTH + grid_x] = true;
  }

  void rebuild_merged_rects() {
    merged_rects.clear();
    std::vector<bool> processed(game_constants::GRID_SIZE, false);

    for (int grid_y = 0; grid_y < game_constants::GRID_HEIGHT; ++grid_y) {
      for (int grid_x = 0; grid_x < game_constants::GRID_WIDTH; ++grid_x) {
        int idx = grid_y * game_constants::GRID_WIDTH + grid_x;
        if (processed[idx] || !is_revealed(grid_x, grid_y)) {
          continue;
        }

        int rect_width = 1;
        int rect_height = 1;

        while (grid_x + rect_width < game_constants::GRID_WIDTH &&
               is_revealed(grid_x + rect_width, grid_y) &&
               !processed[grid_y * game_constants::GRID_WIDTH + grid_x +
                          rect_width]) {
          rect_width++;
        }

        bool can_extend_down = true;
        while (can_extend_down &&
               grid_y + rect_height < game_constants::GRID_HEIGHT) {
          for (int x = 0; x < rect_width; ++x) {
            if (!is_revealed(grid_x + x, grid_y + rect_height) ||
                processed[(grid_y + rect_height) * game_constants::GRID_WIDTH +
                          grid_x + x]) {
              can_extend_down = false;
              break;
            }
          }
          if (can_extend_down) {
            rect_height++;
          }
        }

        for (int y = 0; y < rect_height; ++y) {
          for (int x = 0; x < rect_width; ++x) {
            processed[(grid_y + y) * game_constants::GRID_WIDTH + grid_x + x] =
                true;
          }
        }

        RevealedRect rect;
        rect.x = game_constants::BRICK_START_X +
                 grid_x * game_constants::BRICK_CELL_SIZE;
        rect.y = game_constants::BRICK_START_Y +
                 grid_y * game_constants::BRICK_CELL_SIZE;
        rect.width = rect_width * game_constants::BRICK_CELL_SIZE;
        rect.height = rect_height * game_constants::BRICK_CELL_SIZE;
        merged_rects.push_back(rect);
      }
    }
  }
};
