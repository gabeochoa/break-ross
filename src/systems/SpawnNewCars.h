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
    if (!shop) {
      log_warn("SpawnNewCars: shop is null");
      return false;
    }

    if (shop->car_count <= last_car_count) {
      return false;
    }

    bool has_cars = false;
    int car_count_found = 0;
    for ([[maybe_unused]] const Transform &transform :
         afterhours::EntityQuery()
             .whereHasTag(ColliderTag::Circle)
             .whereHasComponent<Transform>()
             .gen_as<Transform>()) {
      has_cars = true;
      car_count_found++;
    }

    log_info("SpawnNewCars::should_run: car_count={}, last_car_count={}, "
             "has_cars={}, cars_found={}",
             shop->car_count, last_car_count, has_cars, car_count_found);
    
    if (shop->car_count > 0 && !has_cars) {
      log_info("SpawnNewCars::should_run: no cars exist but car_count > 0, "
               "will spawn at default position");
      return true;
    }
    
    return has_cars;
  }

  virtual void once(float) override {
    IsShopManager *shop =
        afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
    if (shop) {
      int cars_found = 0;
      for ([[maybe_unused]] const Transform &transform :
           afterhours::EntityQuery()
               .whereHasTag(ColliderTag::Circle)
               .whereHasComponent<Transform>()
               .gen_as<Transform>()) {
        cars_found++;
      }
      
      if (cars_found == 0 && shop->car_count == 1) {
        last_car_count = 1;
        log_info("SpawnNewCars::once: initializing last_car_count=1 to prevent "
                 "ghost car spawn on start");
      } else if (shop->car_count <= last_car_count) {
        last_car_count = shop->car_count;
        log_info("SpawnNewCars::once: initialized last_car_count={} (cars_found={}, car_count={})",
                 last_car_count, cars_found, shop->car_count);
      } else {
        log_info("SpawnNewCars::once: car_count={} > last_car_count={}, "
                 "keeping last_car_count to allow spawning in for_each_with",
                 shop->car_count, last_car_count);
      }
    } else {
      log_warn("SpawnNewCars::once: shop is null");
    }
  }

  virtual void for_each_with(afterhours::Entity &, IsShopManager &shop,
                             float) override {

    int cars_to_spawn = shop.car_count - last_car_count;
    log_info("SpawnNewCars::for_each_with: car_count={}, last_car_count={}, "
             "cars_to_spawn={}",
             shop.car_count, last_car_count, cars_to_spawn);

    last_car_count = shop.car_count;

    Transform *existing_car_transform_ptr = nullptr;
    int cars_found = 0;
    for (Transform &transform : afterhours::EntityQuery()
                                    .whereHasTag(ColliderTag::Circle)
                                    .whereHasComponent<Transform>()
                                    .gen_as<Transform>()) {
      if (!existing_car_transform_ptr) {
        existing_car_transform_ptr = &transform;
      }
      cars_found++;
    }

    log_info("SpawnNewCars: found {} existing cars", cars_found);

    float radius = 6.0f;
    int damage = shop.get_car_damage_value();
    vec2 spawn_position;
    vec2 base_velocity;

    if (!existing_car_transform_ptr) {
      RoadNetwork *road_network =
          afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
      
      if (road_network && !road_network->segments.empty()) {
        std::vector<size_t> explored_segments;
        for (size_t i = 0; i < road_network->segments.size(); ++i) {
          if (MapRevealSystem::query_segment(i)) {
            explored_segments.push_back(i);
          }
        }
        
        if (!explored_segments.empty()) {
          static std::mt19937 rng(std::random_device{}());
          std::uniform_int_distribution<size_t> seg_dist(0, explored_segments.size() - 1);
          size_t chosen_seg = explored_segments[seg_dist(rng)];
          spawn_position = road_network->segments[chosen_seg].start;
          vec2 seg_dir = {road_network->segments[chosen_seg].end.x - road_network->segments[chosen_seg].start.x,
                          road_network->segments[chosen_seg].end.y - road_network->segments[chosen_seg].start.y};
          float seg_len = std::sqrt(seg_dir.x * seg_dir.x + seg_dir.y * seg_dir.y);
          if (seg_len > 0.1f) {
            seg_dir.x /= seg_len;
            seg_dir.y /= seg_len;
            base_velocity = {seg_dir.x * 200.0f, seg_dir.y * 200.0f};
          } else {
            base_velocity = {200.0f, 200.0f};
          }
          log_info("SpawnNewCars: spawning at explored segment {} position ({:.1f}, {:.1f})",
                   chosen_seg, spawn_position.x, spawn_position.y);
        } else {
          spawn_position = {game_constants::WORLD_WIDTH * 0.5f,
                           game_constants::WORLD_HEIGHT * 0.5f};
          base_velocity = {200.0f, 200.0f};
          log_info("SpawnNewCars: no explored segments found, using default position");
        }
      } else {
        spawn_position = {game_constants::WORLD_WIDTH * 0.5f,
                         game_constants::WORLD_HEIGHT * 0.5f};
        base_velocity = {200.0f, 200.0f};
        log_info("SpawnNewCars: no road network or fog, using default position");
      }
    } else {
      Transform &existing_car_transform = *existing_car_transform_ptr;
      spawn_position = existing_car_transform.position;
      base_velocity = existing_car_transform.velocity;
    }

    log_info("SpawnNewCars: existing car at ({:.1f}, {:.1f}), velocity ({:.1f}, "
             "{:.1f})",
             spawn_position.x, spawn_position.y, base_velocity.x,
             base_velocity.y);

    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angle_dist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> speed_variation(0.7f, 1.3f);

    float base_speed = std::sqrt(base_velocity.x * base_velocity.x +
                                 base_velocity.y * base_velocity.y);
    if (base_speed < 0.1f) {
      log_info("SpawnNewCars: base speed too low ({:.1f}), using default",
               base_speed);
      base_velocity = {200.0f, 200.0f};
      base_speed = std::sqrt(base_velocity.x * base_velocity.x +
                             base_velocity.y * base_velocity.y);
    }

    float base_angle = std::atan2(base_velocity.y, base_velocity.x);

    std::uniform_real_distribution<float> position_offset_dist(-radius * 0.5f,
                                                                radius * 0.5f);

    log_info("SpawnNewCars: spawning {} cars with base_speed={:.1f}, "
             "base_angle={:.2f}",
             cars_to_spawn, base_speed, base_angle);

    for (int i = 0; i < cars_to_spawn; ++i) {
      float angle_offset = angle_dist(rng);
      float speed_mult = speed_variation(rng);
      float new_angle = base_angle + angle_offset;
      float new_speed = base_speed * speed_mult;
      vec2 varied_velocity = {std::cos(new_angle) * new_speed,
                              std::sin(new_angle) * new_speed};

      vec2 offset_position = spawn_position;
      offset_position.x += position_offset_dist(rng);
      offset_position.y += position_offset_dist(rng);

      log_info("SpawnNewCars: spawning car {} at ({:.1f}, {:.1f}) with "
               "velocity ({:.1f}, {:.1f}), speed={:.1f}, angle={:.2f}",
               i + 1, offset_position.x, offset_position.y, varied_velocity.x,
               varied_velocity.y, new_speed, new_angle);

      make_car(offset_position, varied_velocity, radius, damage);
    }

    log_info("SpawnNewCars: finished spawning {} cars", cars_to_spawn);
  }
};

