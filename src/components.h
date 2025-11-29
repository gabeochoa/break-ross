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

  int ball_speed_level{0};
  int ball_damage_level{0};
  int ball_count{1};

  bool shop_open{false};

  IsShopManager() = default;
  IsShopManager(int cost, int damage, int pixels_in)
      : ball_cost(cost), ball_damage(damage), pixels_collected(pixels_in) {}

  int get_upgrade_cost(int base_cost, int level) const {
    return static_cast<int>(base_cost * std::pow(1.5, level));
  }

  int get_ball_speed_cost() const {
    return get_upgrade_cost(50, ball_speed_level);
  }

  int get_ball_damage_cost() const {
    return get_upgrade_cost(500, ball_damage_level);
  }

  int get_new_ball_cost() const {
    return get_upgrade_cost(100, ball_count - 1);
  }

  float get_ball_speed_multiplier() const {
    return 1.0f + (ball_speed_level * 0.2f);
  }

  int get_ball_damage_value() const {
    return ball_damage + (ball_damage_level * 1);
  }

  bool purchase_ball_speed() {
    int cost = get_ball_speed_cost();
    if (pixels_collected >= cost) {
      pixels_collected -= cost;
      ball_speed_level++;
      return true;
    }
    return false;
  }

  bool purchase_ball_damage() {
    int cost = get_ball_damage_cost();
    if (pixels_collected >= cost) {
      pixels_collected -= cost;
      ball_damage_level++;
      return true;
    }
    return false;
  }

  bool purchase_new_ball() {
    int cost = get_new_ball_cost();
    if (pixels_collected >= cost) {
      pixels_collected -= cost;
      ball_count++;
      return true;
    }
    return false;
  }
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
  raylib::Texture2D photo_texture{};
  bool is_loaded{false};
  float reveal_percentage{0.0f};
  bool merged_rects_dirty{false};

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
    int idx = grid_y * game_constants::GRID_WIDTH + grid_x;
    if (!revealed_cells[idx]) {
      revealed_cells[idx] = true;
      merged_rects_dirty = true;
    }
  }

  void rebuild_merged_rects() {
    if (!merged_rects_dirty) {
      return;
    }
    merged_rects_dirty = false;
    merged_rects.clear();
    std::bitset<game_constants::GRID_SIZE> processed;

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
    update_reveal_percentage();
  }

  float get_reveal_percentage() const {
    int revealed_count = 0;
    for (int i = 0; i < game_constants::GRID_SIZE; ++i) {
      if (revealed_cells[i]) {
        revealed_count++;
      }
    }
    return (static_cast<float>(revealed_count) /
            static_cast<float>(game_constants::GRID_SIZE)) *
           100.0f;
  }

  void update_reveal_percentage() {
    reveal_percentage = get_reveal_percentage();
  }
};

struct MergedBrickRect {
  int grid_x;
  int grid_y;
  int width;
  int height;
};

struct BrickGrid : afterhours::BaseComponent {
  std::array<std::array<uint8_t, 50>, game_constants::GRID_HEIGHT> health_data;
  mutable std::vector<MergedBrickRect> cached_rects;
  mutable bool rects_dirty{true};
  mutable raylib::Texture2D health_texture{};
  mutable bool health_texture_dirty{true};

  BrickGrid() {
    for (auto &row : health_data) {
      row.fill(0);
    }
  }

  short get_health(int grid_x, int grid_y) const {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return 0;
    }
    uint8_t byte = health_data[grid_y][grid_x / 2];
    int shift = (grid_x & 1) * 4;
    return (byte >> shift) & 0x0F;
  }

  void set_health(int grid_x, int grid_y, short health) {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return;
    }
    health =
        static_cast<short>(std::max(0, std::min(15, static_cast<int>(health))));

    uint8_t &byte = health_data[grid_y][grid_x / 2];
    int shift = (grid_x & 1) * 4;
    uint8_t inverse_mask = static_cast<uint8_t>(0xF0 >> shift);
    byte =
        (byte & inverse_mask) | static_cast<uint8_t>((health & 0x0F) << shift);
    rects_dirty = true;
    health_texture_dirty = true;
  }

  void add_health(int grid_x, int grid_y, short delta) {
    short current = get_health(grid_x, grid_y);
    set_health(grid_x, grid_y, current + delta);
  }

  bool has_brick(int grid_x, int grid_y) const {
    return get_health(grid_x, grid_y) > 0;
  }
};
