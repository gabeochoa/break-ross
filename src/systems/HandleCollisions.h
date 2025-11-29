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

struct HandleCollisions
    : afterhours::System<Transform, CanDamage,
                         afterhours::tags::All<ColliderTag::Circle>> {
  mutable BrickGrid *cached_brick_grid{nullptr};
  mutable IsShopManager *cached_shop{nullptr};
  mutable IsPhotoReveal *cached_photo_reveal{nullptr};

  virtual void once(float) override {
    cached_brick_grid =
        afterhours::EntityHelper::get_singleton_cmp<BrickGrid>();
    if (!cached_brick_grid) {
      log_error("BrickGrid singleton not found");
      return;
    }

    cached_shop = afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
    if (!cached_shop) {
      log_error("IsShopManager singleton not found");
      return;
    }

    cached_photo_reveal =
        afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
    if (!cached_photo_reveal) {
      log_error("IsPhotoReveal singleton not found");
      return;
    }
  }

  virtual void for_each_with(afterhours::Entity &, Transform &ball_transform,
                             CanDamage &ball_damage, float) override {
    if (!cached_brick_grid || !cached_shop || !cached_photo_reveal) {
      return;
    }

    float ball_radius = ball_transform.size.x / 2.0f;
    vec2 ball_center = {ball_transform.position.x + ball_radius,
                        ball_transform.position.y + ball_radius};

    const float grid_end_x =
        game_constants::BRICK_START_X +
        game_constants::GRID_WIDTH * game_constants::BRICK_CELL_SIZE;
    const float grid_end_y =
        game_constants::BRICK_START_Y +
        game_constants::GRID_HEIGHT * game_constants::BRICK_CELL_SIZE;

    if (ball_center.x + ball_radius < game_constants::BRICK_START_X ||
        ball_center.x - ball_radius > grid_end_x ||
        ball_center.y + ball_radius < game_constants::BRICK_START_Y ||
        ball_center.y - ball_radius > grid_end_y) {
      return;
    }

    int min_grid_x =
        game_constants::world_to_grid_x(ball_center.x - ball_radius);
    int max_grid_x =
        game_constants::world_to_grid_x(ball_center.x + ball_radius);
    int min_grid_y =
        game_constants::world_to_grid_y(ball_center.y - ball_radius);
    int max_grid_y =
        game_constants::world_to_grid_y(ball_center.y + ball_radius);

    if (min_grid_x < 0)
      min_grid_x = 0;
    if (max_grid_x >= game_constants::GRID_WIDTH)
      max_grid_x = game_constants::GRID_WIDTH - 1;
    if (min_grid_y < 0)
      min_grid_y = 0;
    if (max_grid_y >= game_constants::GRID_HEIGHT)
      max_grid_y = game_constants::GRID_HEIGHT - 1;

    bool ball_inside_any_brick = false;
    vec2 stored_velocity = ball_transform.velocity;

    for (int grid_y = min_grid_y; grid_y <= max_grid_y; ++grid_y) {
      for (int grid_x = min_grid_x; grid_x <= max_grid_x; ++grid_x) {
        if (!cached_brick_grid->has_brick(grid_x, grid_y)) {
          continue;
        }

        float brick_left = game_constants::BRICK_START_X +
                           grid_x * game_constants::BRICK_CELL_SIZE;
        float brick_right = brick_left + game_constants::BRICK_CELL_SIZE;
        float brick_top = game_constants::BRICK_START_Y +
                          grid_y * game_constants::BRICK_CELL_SIZE;
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
          cached_brick_grid->add_health(
              grid_x, grid_y, static_cast<short>(-ball_damage.amount));
          if (cached_brick_grid->get_health(grid_x, grid_y) <= 0) {
            cached_shop->pixels_collected += 1;
            cached_photo_reveal->set_revealed(grid_x, grid_y);
          }
          return;
        }

        if (handle_ball_edge_collision_with_cell(
                ball_center, ball_radius, ball_transform, brick_left,
                brick_right, brick_top, brick_bottom)) {
          cached_brick_grid->add_health(
              grid_x, grid_y, static_cast<short>(-ball_damage.amount));
          if (cached_brick_grid->get_health(grid_x, grid_y) <= 0) {
            cached_shop->pixels_collected += 1;
            cached_photo_reveal->set_revealed(grid_x, grid_y);
          }
          return;
        }
      }
    }

    if (!ball_inside_any_brick) {
      restore_ball_velocity(ball_transform, stored_velocity);
    }
  }
};
