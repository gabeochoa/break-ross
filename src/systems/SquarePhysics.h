#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include <afterhours/ah.h>
#include <algorithm>
#include <cmath>
#include <random>

struct SquarePhysics
    : afterhours::System<Transform, RoadFollowing,
                         afterhours::tags::All<ColliderTag::Square>> {
  virtual void once(float) override {}

  virtual void for_each_with(afterhours::Entity &, Transform &transform,
                             RoadFollowing &road_following, float dt) override {
    RoadNetwork *road_network =
        afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
    if (!road_network || road_network->segments.empty()) {
      return;
    }

    if (road_following.current_segment_index >= road_network->segments.size()) {
      road_following.current_segment_index = 0;
      road_following.progress_along_segment = 0.0f;
    }

    RoadSegment &segment =
        road_network->segments[road_following.current_segment_index];

    vec2 segment_start =
        road_following.reverse_direction ? segment.end : segment.start;
    vec2 segment_end =
        road_following.reverse_direction ? segment.start : segment.end;

    vec2 direction = {segment_end.x - segment_start.x,
                      segment_end.y - segment_start.y};
    float segment_length =
        std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (segment_length < 0.001f) {
      road_following.current_segment_index++;
      road_following.progress_along_segment = 0.0f;
      road_following.reverse_direction = false;
      return;
    }

    float normalized_x = direction.x / segment_length;
    float normalized_y = direction.y / segment_length;

    road_network->mark_visited(road_following.current_segment_index);

    float distance_to_travel = road_following.speed * dt;
    float remaining_on_segment =
        (1.0f - road_following.progress_along_segment) * segment_length;

    if (distance_to_travel >= remaining_on_segment) {
      transform.position = segment_end;
      road_following.progress_along_segment = 0.0f;

      vec2 current_end = segment_end;
      float connection_tolerance = 5.0f;
      size_t next_segment_index = SIZE_MAX;
      bool next_reverse_direction = false;

      for (size_t i = 0; i < road_network->segments.size(); ++i) {
        if (i == road_following.current_segment_index) {
          continue;
        }
        const RoadSegment &candidate = road_network->segments[i];

        float dist_to_start =
            std::sqrt((candidate.start.x - current_end.x) *
                          (candidate.start.x - current_end.x) +
                      (candidate.start.y - current_end.y) *
                          (candidate.start.y - current_end.y));
        float dist_to_end = std::sqrt((candidate.end.x - current_end.x) *
                                          (candidate.end.x - current_end.x) +
                                      (candidate.end.y - current_end.y) *
                                          (candidate.end.y - current_end.y));

        if (dist_to_start < connection_tolerance) {
          next_segment_index = i;
          next_reverse_direction = false;
          break;
        } else if (dist_to_end < connection_tolerance) {
          next_segment_index = i;
          next_reverse_direction = true;
          break;
        }
      }

      if (next_segment_index != SIZE_MAX) {
        road_following.current_segment_index = next_segment_index;
        road_following.reverse_direction = next_reverse_direction;
      } else {
        size_t unvisited_segment =
            road_network->find_random_unvisited_segment();
        if (unvisited_segment != SIZE_MAX) {
          const RoadSegment &target = road_network->segments[unvisited_segment];
          float dist_to_start = std::sqrt((target.start.x - current_end.x) *
                                              (target.start.x - current_end.x) +
                                          (target.start.y - current_end.y) *
                                              (target.start.y - current_end.y));
          float dist_to_end = std::sqrt(
              (target.end.x - current_end.x) * (target.end.x - current_end.x) +
              (target.end.y - current_end.y) * (target.end.y - current_end.y));

          road_following.current_segment_index = unvisited_segment;
          road_following.reverse_direction = dist_to_end < dist_to_start;
          transform.position =
              road_following.reverse_direction ? target.end : target.start;
        } else {
          road_following.current_segment_index++;
          road_following.reverse_direction = false;
          if (road_following.current_segment_index >=
              road_network->segments.size()) {
            road_following.current_segment_index = 0;
          }
        }
      }
    } else {
      road_following.progress_along_segment +=
          distance_to_travel / segment_length;
      transform.position.x =
          segment_start.x + road_following.progress_along_segment * direction.x;
      transform.position.y =
          segment_start.y + road_following.progress_along_segment * direction.y;
    }

    transform.velocity.x = normalized_x * road_following.speed;
    transform.velocity.y = normalized_y * road_following.speed;
  }
};
