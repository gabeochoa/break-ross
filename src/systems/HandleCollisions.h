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
bool is_car_inside_brick_cell(vec2 car_center, float brick_left,
                               float brick_right, float brick_top,
                               float brick_bottom) {
  return car_center.x > brick_left && car_center.x < brick_right &&
         car_center.y > brick_top && car_center.y < brick_bottom;
}

bool handle_car_inside_brick_cell(Transform &car_transform,
                                   vec2 &stored_velocity) {
  if (stored_velocity.x == 0.0f && stored_velocity.y == 0.0f) {
    stored_velocity = {200.0f, 200.0f};
  }
  car_transform.velocity.x = 0.0f;
  car_transform.velocity.y = 0.0f;
  return true;
}

bool handle_car_edge_collision_with_cell(vec2 car_center, float car_radius,
                                          Transform &car_transform,
                                          float brick_left, float brick_right,
                                          float brick_top, float brick_bottom) {
  float closest_x = std::max(brick_left, std::min(car_center.x, brick_right));
  float closest_y = std::max(brick_top, std::min(car_center.y, brick_bottom));

  float dx = car_center.x - closest_x;
  float dy = car_center.y - closest_y;
  float distance_sq = dx * dx + dy * dy;

  if (distance_sq >= car_radius * car_radius) {
    return false;
  }

  float distance = std::sqrt(distance_sq);
  vec2 normal = {dx / distance, dy / distance};

  float dot_product = car_transform.velocity.x * normal.x +
                      car_transform.velocity.y * normal.y;
  car_transform.velocity.x -= 2.0f * dot_product * normal.x;
  car_transform.velocity.y -= 2.0f * dot_product * normal.y;

  return true;
}

void restore_car_velocity(Transform &car_transform, vec2 &stored_velocity) {
  if (car_transform.velocity.x == 0.0f && car_transform.velocity.y == 0.0f) {
    if (stored_velocity.x == 0.0f && stored_velocity.y == 0.0f) {
      stored_velocity = {200.0f, 200.0f};
    }
    car_transform.velocity = stored_velocity;
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

  virtual void for_each_with(afterhours::Entity &, Transform &car_transform,
                             CanDamage &car_damage, float) override {
    if (!cached_brick_grid || !cached_shop || !cached_photo_reveal) {
      return;
    }

    float car_radius = car_transform.size.x / 2.0f;
    vec2 car_center = {car_transform.position.x + car_radius,
                        car_transform.position.y + car_radius};

    const float grid_end_x =
        game_constants::BRICK_START_X +
        game_constants::GRID_WIDTH * game_constants::BRICK_CELL_SIZE;
    const float grid_end_y =
        game_constants::BRICK_START_Y +
        game_constants::GRID_HEIGHT * game_constants::BRICK_CELL_SIZE;

    if (car_center.x + car_radius < game_constants::BRICK_START_X ||
        car_center.x - car_radius > grid_end_x ||
        car_center.y + car_radius < game_constants::BRICK_START_Y ||
        car_center.y - car_radius > grid_end_y) {
      return;
    }

    int min_grid_x =
        game_constants::world_to_grid_x(car_center.x - car_radius);
    int max_grid_x =
        game_constants::world_to_grid_x(car_center.x + car_radius);
    int min_grid_y =
        game_constants::world_to_grid_y(car_center.y - car_radius);
    int max_grid_y =
        game_constants::world_to_grid_y(car_center.y + car_radius);

    if (min_grid_x < 0)
      min_grid_x = 0;
    if (max_grid_x >= game_constants::GRID_WIDTH)
      max_grid_x = game_constants::GRID_WIDTH - 1;
    if (min_grid_y < 0)
      min_grid_y = 0;
    if (max_grid_y >= game_constants::GRID_HEIGHT)
      max_grid_y = game_constants::GRID_HEIGHT - 1;

    bool car_inside_any_brick = false;
    vec2 stored_velocity = car_transform.velocity;

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

        if (car_center.x + car_radius < brick_left ||
            car_center.x - car_radius > brick_right ||
            car_center.y + car_radius < brick_top ||
            car_center.y - car_radius > brick_bottom) {
          continue;
        }

        if (is_car_inside_brick_cell(car_center, brick_left, brick_right,
                                      brick_top, brick_bottom)) {
          car_inside_any_brick = true;
          handle_car_inside_brick_cell(car_transform, stored_velocity);
          cached_brick_grid->add_health(
              grid_x, grid_y, static_cast<short>(-car_damage.amount));
          if (cached_brick_grid->get_health(grid_x, grid_y) <= 0) {
            cached_shop->pixels_collected += 1;
            cached_photo_reveal->set_revealed(grid_x, grid_y);
          }
          return;
        }

        if (handle_car_edge_collision_with_cell(
                car_center, car_radius, car_transform, brick_left,
                brick_right, brick_top, brick_bottom)) {
          cached_brick_grid->add_health(
              grid_x, grid_y, static_cast<short>(-car_damage.amount));
          if (cached_brick_grid->get_health(grid_x, grid_y) <= 0) {
            cached_shop->pixels_collected += 1;
            cached_photo_reveal->set_revealed(grid_x, grid_y);
          }
          return;
        }
      }
    }

    if (!car_inside_any_brick) {
      restore_car_velocity(car_transform, stored_velocity);
    }
  }
};
