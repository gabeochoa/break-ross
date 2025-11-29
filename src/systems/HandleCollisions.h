#pragma once

#include "../components.h"
#include "../eq.h"
#include <afterhours/ah.h>
#include <algorithm>
#include <cmath>

namespace {
bool is_ball_inside_brick(vec2 ball_center, float brick_left, float brick_right,
                          float brick_top, float brick_bottom) {
  return ball_center.x > brick_left && ball_center.x < brick_right &&
         ball_center.y > brick_top && ball_center.y < brick_bottom;
}

bool handle_ball_inside_brick(Transform &ball_transform,
                              HasHealth &brick_health,
                              afterhours::Entity &brick_entity, int damage,
                              vec2 &stored_velocity) {
  if (stored_velocity.x == 0.0f && stored_velocity.y == 0.0f) {
    stored_velocity = {200.0f, 200.0f};
  }
  ball_transform.velocity.x = 0.0f;
  ball_transform.velocity.y = 0.0f;
  brick_health.amount -= damage;
  brick_entity.cleanup = !brick_health.amount;
  return true;
}

bool handle_ball_edge_collision(vec2 ball_center, float ball_radius,
                                Transform &ball_transform,
                                HasHealth &brick_health,
                                afterhours::Entity &brick_entity, int damage,
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

  brick_health.amount -= damage;
  brick_entity.cleanup = !brick_health.amount;

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
  virtual void once(float) override {
    auto balls = EQ().whereHasComponent<Transform>()
                     .whereHasTag(ColliderTag::Circle)
                     .whereHasComponent<CanDamage>()
                     .gen();

    for (afterhours::Entity &ball_entity : balls) {
      Transform &ball_transform = ball_entity.get<Transform>();
      CanDamage &ball_damage = ball_entity.get<CanDamage>();

      float ball_radius = ball_transform.size.x / 2.0f;
      vec2 ball_center = {ball_transform.position.x + ball_radius,
                          ball_transform.position.y + ball_radius};

      auto nearby_bricks = EQ().whereHasComponent<Transform>()
                               .whereHasTag(ColliderTag::Rect)
                               .whereHasComponent<HasHealth>()
                               .whereNearby(ball_center, ball_radius * 2.0f)
                               .whereLambda([](const afterhours::Entity &e) {
                                 return e.get<HasHealth>().amount > 0;
                               })
                               .gen();

      bool ball_inside_any_brick = false;
      vec2 stored_velocity = ball_transform.velocity;

      for (afterhours::Entity &brick_entity : nearby_bricks) {
        Transform &brick_transform = brick_entity.get<Transform>();
        HasHealth &brick_health = brick_entity.get<HasHealth>();

        float brick_left = brick_transform.position.x;
        float brick_right = brick_transform.position.x + brick_transform.size.x;
        float brick_top = brick_transform.position.y;
        float brick_bottom =
            brick_transform.position.y + brick_transform.size.y;

        if (ball_center.x + ball_radius < brick_left ||
            ball_center.x - ball_radius > brick_right ||
            ball_center.y + ball_radius < brick_top ||
            ball_center.y - ball_radius > brick_bottom) {
          continue;
        }

        if (is_ball_inside_brick(ball_center, brick_left, brick_right,
                                 brick_top, brick_bottom)) {
          ball_inside_any_brick = true;
          handle_ball_inside_brick(ball_transform, brick_health, brick_entity,
                                   ball_damage.amount, stored_velocity);
          break;
        }

        if (handle_ball_edge_collision(ball_center, ball_radius, ball_transform,
                                       brick_health, brick_entity,
                                       ball_damage.amount, brick_left,
                                       brick_right, brick_top, brick_bottom)) {
          break;
        }
      }

      if (!ball_inside_any_brick) {
        restore_ball_velocity(ball_transform, stored_velocity);
      }
    }
  }
};
