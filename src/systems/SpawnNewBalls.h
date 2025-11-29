#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../game_setup.h"
#include "../render_backend.h"
#include "../settings.h"
#include <afterhours/ah.h>
#include <random>

struct SpawnNewBalls : afterhours::System<IsShopManager> {
  int last_ball_count{0};

  virtual bool should_run(float) override {
    IsShopManager *shop =
        afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
    return shop->ball_count > last_ball_count;
  }

  virtual void once(float) override {}

  virtual void for_each_with(afterhours::Entity &, IsShopManager &shop,
                             float) override {

    int balls_to_spawn = shop.ball_count - last_ball_count;
    last_ball_count = shop.ball_count;

    Transform &existing_ball_transform = afterhours::EntityQuery()
                                             .whereHasTag(ColliderTag::Circle)
                                             .whereHasComponent<Transform>()
                                             .gen_first_as<Transform>();

    float radius = 10.0f;
    int damage = shop.get_ball_damage_value();
    vec2 spawn_position = existing_ball_transform.position;
    vec2 base_velocity = existing_ball_transform.velocity;

    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angle_dist(-0.3f, 0.3f);
    std::uniform_real_distribution<float> speed_variation(0.9f, 1.1f);

    for (int i = 0; i < balls_to_spawn; ++i) {
      float angle = angle_dist(rng);
      float speed_mult = speed_variation(rng);
      float cos_a = std::cos(angle);
      float sin_a = std::sin(angle);
      vec2 varied_velocity = {
          (base_velocity.x * cos_a - base_velocity.y * sin_a) * speed_mult,
          (base_velocity.x * sin_a + base_velocity.y * cos_a) * speed_mult};
      make_ball(spawn_position, varied_velocity, radius, damage);
    }
  }
};
