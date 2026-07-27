#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../game_setup.h"
#include "../log.h"
#include "../render_backend.h"
#include "../settings.h"
#include "MapRevealSystem.h"
#include <afterhours/ah.h>
#include <random>

struct SpawnNewCars : afterhours::System<IsShopManager> {
  int last_car_count{0};

  virtual bool should_run(float) override {
    IsShopManager *shop =
        afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
    invariant(shop, "IsShopManager singleton not found");

    return shop->car_count > last_car_count;
  }

  virtual void once(float) override {
    IsShopManager *shop =
        afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
    invariant(shop, "IsShopManager singleton not found");

    int cars_found = 0;
    for ([[maybe_unused]] const Transform &transform :
         afterhours::EntityQuery()
             .whereHasTag(ColliderTag::Circle)
             .whereHasComponent<Transform>()
             .gen_as<Transform>()) {
      cars_found++;
    }

    // Prevent a ghost car spawn on a fresh start, and keep last_car_count in
    // sync when we're not behind. Otherwise leave it so for_each_with spawns
    // the difference (car_count - last_car_count).
    if (cars_found == 0 && shop->car_count == 1) {
      last_car_count = 1;
    } else if (shop->car_count <= last_car_count) {
      last_car_count = shop->car_count;
    }
  }

  virtual void for_each_with(afterhours::Entity &, IsShopManager &shop,
                             float) override {
    int cars_to_spawn = shop.car_count - last_car_count;
    last_car_count = shop.car_count;

    Transform *existing_car_transform_ptr = nullptr;
    for (Transform &transform : afterhours::EntityQuery()
                                    .whereHasTag(ColliderTag::Circle)
                                    .whereHasComponent<Transform>()
                                    .gen_as<Transform>()) {
      existing_car_transform_ptr = &transform;
      break;
    }

    float radius = 6.0f;
    int damage = shop.get_car_damage_value();
    vec2 spawn_position;
    vec2 base_velocity;

    if (!existing_car_transform_ptr) {
      RoadNetwork *road_network =
          afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
      invariant(road_network, "RoadNetwork singleton not found");

      if (!road_network->is_loaded || road_network->segments.empty()) {
        spawn_position = {game_constants::WORLD_WIDTH * 0.5f,
                          game_constants::WORLD_HEIGHT * 0.5f};
        base_velocity = {200.0f, 200.0f};
      } else {
        std::vector<size_t> explored_segments;
        for (size_t i = 0; i < road_network->segments.size(); ++i) {
          if (MapRevealSystem::query_segment(i)) {
            explored_segments.push_back(i);
          }
        }

        if (!explored_segments.empty()) {
          static std::mt19937 rng(std::random_device{}());
          std::uniform_int_distribution<size_t> seg_dist(
              0, explored_segments.size() - 1);
          size_t chosen_seg = explored_segments[seg_dist(rng)];
          spawn_position = road_network->segments[chosen_seg].start;
          vec2 seg_dir = {road_network->segments[chosen_seg].end.x -
                              road_network->segments[chosen_seg].start.x,
                          road_network->segments[chosen_seg].end.y -
                              road_network->segments[chosen_seg].start.y};
          float seg_len =
              std::sqrt(seg_dir.x * seg_dir.x + seg_dir.y * seg_dir.y);
          if (seg_len > 0.1f) {
            seg_dir.x /= seg_len;
            seg_dir.y /= seg_len;
            base_velocity = {seg_dir.x * 200.0f, seg_dir.y * 200.0f};
          } else {
            base_velocity = {200.0f, 200.0f};
          }
        } else {
          spawn_position = {game_constants::WORLD_WIDTH * 0.5f,
                            game_constants::WORLD_HEIGHT * 0.5f};
          base_velocity = {200.0f, 200.0f};
        }
      }
    } else {
      Transform &existing_car_transform = *existing_car_transform_ptr;
      spawn_position = existing_car_transform.position;
      base_velocity = existing_car_transform.velocity;
    }

    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angle_dist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> speed_variation(0.7f, 1.3f);

    float base_speed = std::sqrt(base_velocity.x * base_velocity.x +
                                 base_velocity.y * base_velocity.y);
    if (base_speed < 0.1f) {
      base_velocity = {200.0f, 200.0f};
      base_speed = std::sqrt(base_velocity.x * base_velocity.x +
                             base_velocity.y * base_velocity.y);
    }

    float base_angle = std::atan2(base_velocity.y, base_velocity.x);

    std::uniform_real_distribution<float> position_offset_dist(-radius * 0.5f,
                                                               radius * 0.5f);

    for (int i = 0; i < cars_to_spawn; ++i) {
      float new_angle = base_angle + angle_dist(rng);
      float new_speed = base_speed * speed_variation(rng);
      vec2 varied_velocity = {std::cos(new_angle) * new_speed,
                              std::sin(new_angle) * new_speed};

      vec2 offset_position = spawn_position;
      offset_position.x += position_offset_dist(rng);
      offset_position.y += position_offset_dist(rng);

      make_car(offset_position, varied_velocity, radius, damage);
    }
  }
};
