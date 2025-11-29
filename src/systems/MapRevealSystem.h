#pragma once

#include "../components.h"
#include "../game_constants.h"
#include "../log.h"
#include <afterhours/ah.h>
#include <cmath>

struct MapRevealSystem {
  static bool reveal_segment(size_t segment_index) {
    RoadNetwork *road_network =
        afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
    FogOfWar *fog = afterhours::EntityHelper::get_singleton_cmp<FogOfWar>();

    if (!road_network || !road_network->is_loaded ||
        segment_index >= road_network->segments.size()) {
      return false;
    }

    if (road_network->is_visited(segment_index)) {
      return false;
    }

    road_network->mark_visited(segment_index);

    IsShopManager *shop =
        afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
    if (shop) {
      shop->pixels_collected += 1;
    }

    if (!fog) {
      return true;
    }

    const RoadSegment &segment = road_network->segments[segment_index];
    reveal_segment_fog(segment, fog);

    return true;
  }

  static bool query_segment(size_t segment_index) {
    RoadNetwork *road_network =
        afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
    FogOfWar *fog = afterhours::EntityHelper::get_singleton_cmp<FogOfWar>();

    if (!road_network || !road_network->is_loaded ||
        segment_index >= road_network->segments.size()) {
      return false;
    }

    if (road_network->is_visited(segment_index)) {
      return true;
    }

    if (!fog) {
      return false;
    }

    const RoadSegment &segment = road_network->segments[segment_index];
    return is_segment_revealed_in_fog(segment, fog);
  }

  static void reveal_position(const vec2 &position, float radius) {
    FogOfWar *fog = afterhours::EntityHelper::get_singleton_cmp<FogOfWar>();
    if (!fog) {
      return;
    }

    IsPhotoReveal *photo_reveal =
        afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();

    float reveal_radius_sq = radius * radius;
    int center_grid_x = game_constants::world_to_grid_x(position.x);
    int center_grid_y = game_constants::world_to_grid_y(position.y);

    int radius_in_cells = static_cast<int>(
        std::ceil(radius / game_constants::BRICK_CELL_SIZE));

    for (int dy = -radius_in_cells; dy <= radius_in_cells; ++dy) {
      for (int dx = -radius_in_cells; dx <= radius_in_cells; ++dx) {
        int grid_x = center_grid_x + dx;
        int grid_y = center_grid_y + dy;

        if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
            grid_y >= game_constants::GRID_HEIGHT) {
          continue;
        }

        if (fog->is_revealed(grid_x, grid_y)) {
          continue;
        }

        vec2 cell_world_pos = game_constants::grid_to_world_pos(grid_x, grid_y);
        vec2 cell_center = {
            cell_world_pos.x + game_constants::BRICK_CELL_SIZE * 0.5f,
            cell_world_pos.y + game_constants::BRICK_CELL_SIZE * 0.5f};

        float dx_world = cell_center.x - position.x;
        float dy_world = cell_center.y - position.y;
        float dist_sq = dx_world * dx_world + dy_world * dy_world;

        if (dist_sq <= reveal_radius_sq) {
          fog->set_revealed(grid_x, grid_y);
          if (photo_reveal) {
            photo_reveal->set_revealed(grid_x, grid_y);
          }
        }
      }
    }
  }

  static void compute_reachable_cells() {
    RoadNetwork *road_network =
        afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
    FogOfWar *fog = afterhours::EntityHelper::get_singleton_cmp<FogOfWar>();

    if (!road_network || !road_network->is_loaded || !fog) {
      return;
    }

    if (fog->reachable_computed) {
      return;
    }

    float reveal_radius = fog->reveal_radius;

    for (const RoadSegment &segment : road_network->segments) {
      vec2 start = segment.start;
      vec2 end = segment.end;
      vec2 direction = {end.x - start.x, end.y - start.y};
      float segment_length = std::sqrt(direction.x * direction.x +
                                       direction.y * direction.y);

      if (segment_length < 0.001f) {
        mark_reachable_position(start, reveal_radius, fog);
        continue;
      }

      int steps = static_cast<int>(std::ceil(segment_length / reveal_radius)) + 1;

      for (int i = 0; i <= steps; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        vec2 pos = {start.x + direction.x * t, start.y + direction.y * t};
        mark_reachable_position(pos, reveal_radius, fog);
      }
    }

    fog->reachable_computed = true;
  }

private:
  static void mark_reachable_position(const vec2 &position, float radius,
                                      FogOfWar *fog) {
    float reveal_radius_sq = radius * radius;
    int center_grid_x = game_constants::world_to_grid_x(position.x);
    int center_grid_y = game_constants::world_to_grid_y(position.y);

    int radius_in_cells = static_cast<int>(
        std::ceil(radius / game_constants::BRICK_CELL_SIZE));

    for (int dy = -radius_in_cells; dy <= radius_in_cells; ++dy) {
      for (int dx = -radius_in_cells; dx <= radius_in_cells; ++dx) {
        int grid_x = center_grid_x + dx;
        int grid_y = center_grid_y + dy;

        if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
            grid_y >= game_constants::GRID_HEIGHT) {
          continue;
        }

        if (fog->is_reachable(grid_x, grid_y)) {
          continue;
        }

        vec2 cell_world_pos = game_constants::grid_to_world_pos(grid_x, grid_y);
        vec2 cell_center = {
            cell_world_pos.x + game_constants::BRICK_CELL_SIZE * 0.5f,
            cell_world_pos.y + game_constants::BRICK_CELL_SIZE * 0.5f};

        float dx_world = cell_center.x - position.x;
        float dy_world = cell_center.y - position.y;
        float dist_sq = dx_world * dx_world + dy_world * dy_world;

        if (dist_sq <= reveal_radius_sq) {
          fog->set_reachable(grid_x, grid_y);
        }
      }
    }
  }

  static void reveal_segment_fog(const RoadSegment &segment, FogOfWar *fog) {
    vec2 start = segment.start;
    vec2 end = segment.end;
    vec2 direction = {end.x - start.x, end.y - start.y};
    float segment_length = std::sqrt(direction.x * direction.x +
                                     direction.y * direction.y);

    if (segment_length < 0.001f) {
      reveal_position(start, fog->reveal_radius);
      return;
    }

    float reveal_radius = fog->reveal_radius;
    int steps = static_cast<int>(std::ceil(segment_length / reveal_radius)) + 1;

    for (int i = 0; i <= steps; ++i) {
      float t = static_cast<float>(i) / static_cast<float>(steps);
      vec2 pos = {start.x + direction.x * t, start.y + direction.y * t};
      reveal_position(pos, reveal_radius);
    }
  }

  static bool is_segment_revealed_in_fog(const RoadSegment &segment,
                                         FogOfWar *fog) {
    vec2 start = segment.start;
    vec2 end = segment.end;
    vec2 direction = {end.x - start.x, end.y - start.y};
    float segment_length = std::sqrt(direction.x * direction.x +
                                     direction.y * direction.y);

    if (segment_length < 0.001f) {
      int grid_x = game_constants::world_to_grid_x(start.x);
      int grid_y = game_constants::world_to_grid_y(start.y);
      return fog->is_revealed(grid_x, grid_y);
    }

    float reveal_radius = fog->reveal_radius;
    int steps = static_cast<int>(std::ceil(segment_length / reveal_radius)) + 1;

    bool any_revealed = false;
    for (int i = 0; i <= steps; ++i) {
      float t = static_cast<float>(i) / static_cast<float>(steps);
      vec2 pos = {start.x + direction.x * t, start.y + direction.y * t};

      int grid_x = game_constants::world_to_grid_x(pos.x);
      int grid_y = game_constants::world_to_grid_y(pos.y);

      if (grid_x >= 0 && grid_x < game_constants::GRID_WIDTH && grid_y >= 0 &&
          grid_y < game_constants::GRID_HEIGHT) {
        if (fog->is_revealed(grid_x, grid_y)) {
          any_revealed = true;
          break;
        }
      }
    }

    return any_revealed;
  }
};

