#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../log.h"
#include <afterhours/ah.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
bool is_ball_inside_brick_cell(vec2 ball_center, float brick_left,
                               float brick_right, float brick_top,
                               float brick_bottom) {
  return ball_center.x > brick_left && ball_center.x < brick_right &&
         ball_center.y > brick_top && ball_center.y < brick_bottom;
}

bool handle_ball_inside_brick_cell(Transform &ball_transform,
                                   vec2 &stored_velocity) {
  if (stored_velocity.x == 0.0f && stored_velocity.y == 0.0f) {
    stored_velocity = {200.0f, 200.0f};
  }
  ball_transform.velocity.x = 0.0f;
  ball_transform.velocity.y = 0.0f;
  return true;
}

bool handle_ball_edge_collision_with_cell(vec2 ball_center, float ball_radius,
                                          Transform &ball_transform,
                                          float brick_left, float brick_right,
                                          float brick_top, float brick_bottom) {
  float closest_x = std::max(brick_left, std::min(ball_center.x, brick_right));
  float closest_y = std::max(brick_top, std::min(ball_center.y, brick_bottom));

  float dx = ball_center.x - closest_x;
  float dy = ball_center.y - closest_y;
  float distance_sq = dx * dx + dy * dy;

  if (distance_sq >= ball_radius * ball_radius) {
    return false;
  }

  float distance = std::sqrt(distance_sq);
  vec2 normal = {dx / distance, dy / distance};

  float dot_product = ball_transform.velocity.x * normal.x +
                      ball_transform.velocity.y * normal.y;
  ball_transform.velocity.x -= 2.0f * dot_product * normal.x;
  ball_transform.velocity.y -= 2.0f * dot_product * normal.y;

  return true;
}

void restore_ball_velocity(Transform &ball_transform, vec2 &stored_velocity) {
  if (ball_transform.velocity.x == 0.0f && ball_transform.velocity.y == 0.0f) {
    if (stored_velocity.x == 0.0f && stored_velocity.y == 0.0f) {
      stored_velocity = {200.0f, 200.0f};
    }
    ball_transform.velocity = stored_velocity;
  }
}
} // namespace

struct HandleCollisions : afterhours::System<> {
  mutable std::vector<afterhours::Entity *> cached_balls;
  mutable bool balls_cache_valid{false};

  virtual void once(float) override {
    BrickGrid *brick_grid =
        afterhours::EntityHelper::get_singleton_cmp<BrickGrid>();
    if (!brick_grid) {
      log_error("BrickGrid singleton not found");
      return;
    }

    IsShopManager *shop =
        afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
    if (!shop) {
      log_error("IsShopManager singleton not found");
      return;
    }

    IsPhotoReveal *photo_reveal =
        afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
    if (!photo_reveal) {
      log_error("IsPhotoReveal singleton not found");
      return;
    }

    if (!balls_cache_valid) {
      cached_balls.clear();
      auto balls = EQ().whereHasComponent<Transform>()
                       .whereHasTag(ColliderTag::Circle)
                       .whereHasComponent<CanDamage>()
                       .gen();
      for (afterhours::Entity &ball : balls) {
        cached_balls.push_back(&ball);
      }
      balls_cache_valid = true;
    }

    for (afterhours::Entity *ball_ptr : cached_balls) {
      afterhours::Entity &ball_entity = *ball_ptr;
      Transform &ball_transform = ball_entity.get<Transform>();
      CanDamage &ball_damage = ball_entity.get<CanDamage>();

      float ball_radius = ball_transform.size.x / 2.0f;
      vec2 ball_center = {ball_transform.position.x + ball_radius,
                          ball_transform.position.y + ball_radius};

      int min_grid_x =
          game_constants::world_to_grid_x(ball_center.x - ball_radius);
      int max_grid_x =
          game_constants::world_to_grid_x(ball_center.x + ball_radius);
      int min_grid_y =
          game_constants::world_to_grid_y(ball_center.y - ball_radius);
      int max_grid_y =
          game_constants::world_to_grid_y(ball_center.y + ball_radius);

      min_grid_x = std::max(0, min_grid_x);
      max_grid_x = std::min(game_constants::GRID_WIDTH - 1, max_grid_x);
      min_grid_y = std::max(0, min_grid_y);
      max_grid_y = std::min(game_constants::GRID_HEIGHT - 1, max_grid_y);

      bool ball_inside_any_brick = false;
      vec2 stored_velocity = ball_transform.velocity;

      for (int grid_y = min_grid_y; grid_y <= max_grid_y; ++grid_y) {
        for (int grid_x = min_grid_x; grid_x <= max_grid_x; ++grid_x) {
          if (!brick_grid->has_brick(grid_x, grid_y)) {
            continue;
          }

          vec2 cell_world_pos =
              game_constants::grid_to_world_pos(grid_x, grid_y);
          float brick_left = cell_world_pos.x;
          float brick_right = brick_left + game_constants::BRICK_CELL_SIZE;
          float brick_top = cell_world_pos.y;
          float brick_bottom = brick_top + game_constants::BRICK_CELL_SIZE;

          if (ball_center.x + ball_radius < brick_left ||
              ball_center.x - ball_radius > brick_right ||
              ball_center.y + ball_radius < brick_top ||
              ball_center.y - ball_radius > brick_bottom) {
            continue;
          }

          if (is_ball_inside_brick_cell(ball_center, brick_left, brick_right,
                                        brick_top, brick_bottom)) {
            ball_inside_any_brick = true;
            handle_ball_inside_brick_cell(ball_transform, stored_velocity);
            brick_grid->add_health(grid_x, grid_y,
                                   static_cast<short>(-ball_damage.amount));
            if (brick_grid->get_health(grid_x, grid_y) <= 0) {
              shop->pixels_collected += 1;
              photo_reveal->set_revealed(grid_x, grid_y);
            }
            goto next_ball;
          }

          if (handle_ball_edge_collision_with_cell(
                  ball_center, ball_radius, ball_transform, brick_left,
                  brick_right, brick_top, brick_bottom)) {
            brick_grid->add_health(grid_x, grid_y,
                                   static_cast<short>(-ball_damage.amount));
            if (brick_grid->get_health(grid_x, grid_y) <= 0) {
              shop->pixels_collected += 1;
              photo_reveal->set_revealed(grid_x, grid_y);
            }
            goto next_ball;
          }
        }
      }

      if (!ball_inside_any_brick) {
        restore_ball_velocity(ball_transform, stored_velocity);
      }

    next_ball:;
    }
  }
};
