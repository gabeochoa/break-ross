#pragma once

#include "../components.h"
#include "../eq.h"
#include <afterhours/ah.h>
#include <algorithm>
#include <cmath>

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

      float ball_left = ball_transform.position.x;
      float ball_right = ball_transform.position.x + ball_transform.size.x;
      float ball_top = ball_transform.position.y;
      float ball_bottom = ball_transform.position.y + ball_transform.size.y;

      auto nearby_bricks = EQ().whereHasComponent<Transform>()
                               .whereHasTag(ColliderTag::Rect)
                               .whereHasComponent<HasHealth>()
                               .whereNearby(ball_center, ball_radius * 2.0f)
                               .whereLambda([](const afterhours::Entity &e) {
                                 return e.get<HasHealth>().amount > 0;
                               })
                               .gen();

      for (afterhours::Entity &brick_entity : nearby_bricks) {
        Transform &brick_transform = brick_entity.get<Transform>();
        HasHealth &brick_health = brick_entity.get<HasHealth>();

        float brick_left = brick_transform.position.x;
        float brick_right = brick_transform.position.x + brick_transform.size.x;
        float brick_top = brick_transform.position.y;
        float brick_bottom =
            brick_transform.position.y + brick_transform.size.y;

        if (ball_right < brick_left || ball_left > brick_right ||
            ball_bottom < brick_top || ball_top > brick_bottom) {
          continue;
        }

        float closest_x =
            std::max(brick_left, std::min(ball_center.x, brick_right));
        float closest_y =
            std::max(brick_top, std::min(ball_center.y, brick_bottom));

        float dx = ball_center.x - closest_x;
        float dy = ball_center.y - closest_y;
        float distance_sq = dx * dx + dy * dy;

        if (distance_sq < ball_radius * ball_radius) {
          brick_health.amount -= ball_damage.amount;

          float dx_to_left = std::abs(ball_center.x - brick_left);
          float dx_to_right = std::abs(ball_center.x - brick_right);
          float dy_to_top = std::abs(ball_center.y - brick_top);
          float dy_to_bottom = std::abs(ball_center.y - brick_bottom);

          float min_horizontal = std::min(dx_to_left, dx_to_right);
          float min_vertical = std::min(dy_to_top, dy_to_bottom);

          if (min_horizontal < min_vertical) {
            ball_transform.velocity.x = -ball_transform.velocity.x;
          } else {
            ball_transform.velocity.y = -ball_transform.velocity.y;
          }

          if (brick_health.amount <= 0) {
            brick_entity.cleanup = true;
          }
        }
      }
    }
  }
};
