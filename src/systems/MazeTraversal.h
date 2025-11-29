#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include <afterhours/ah.h>
#include <algorithm>
#include <cmath>
#include <vector>

struct MazeTraversal
    : afterhours::System<Transform, RoadFollowing,
                         afterhours::tags::All<ColliderTag::Square>> {
  static size_t last_segment_index;
  static size_t second_last_segment_index;

  virtual void once(float) override {}

  virtual void for_each_with(afterhours::Entity & /* entity */,
                             Transform &transform,
                             RoadFollowing &road_following, float dt) override {
    RoadNetwork *road_network =
        afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
    if (!road_network || !road_network->is_loaded ||
        road_network->segments.empty()) {
      return;
    }

    // Update algorithm from shop if available
    IsShopManager *shop =
        afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
    invariant(shop, "Shop manager not found");
    road_following.current_algorithm = shop->get_current_algorithm();

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

    // Mark segment as visited and track if it was newly revealed
    bool was_visited =
        road_network->is_visited(road_following.current_segment_index);
    road_network->mark_visited(road_following.current_segment_index);
    bool just_revealed = !was_visited;

    // Update segments_without_reveal counter
    if (just_revealed) {
      road_following.segments_without_reveal = 0;
      road_following.forced_direction_attempts =
          0; // Reset when we reveal a new segment
    } else {
      road_following.segments_without_reveal++;
    }

    // Move along current segment
    float distance_to_travel = road_following.speed * dt;
    float remaining_on_segment =
        (1.0f - road_following.progress_along_segment) * segment_length;

    if (distance_to_travel >= remaining_on_segment) {
      // Reached end of segment, select next segment
      transform.position = segment_end;
      road_following.progress_along_segment = 0.0f;

      vec2 current_end = segment_end;
      // Connection tolerance should match road width (square size = 12.0)
      // Use a small tolerance to only connect segments that actually meet
      float connection_tolerance = 15.0f; // Slightly larger than square size
      size_t next_segment_index = SIZE_MAX;
      bool next_reverse_direction = false;

      // Select next segment based on algorithm or forced direction mode
      if (road_following.forced_direction_steps > 0) {
        // Forced direction mode: choose segment that best matches forced
        // direction
        select_next_forced_direction(road_following, road_network, current_end,
                                     connection_tolerance, next_segment_index,
                                     next_reverse_direction);
        if (next_segment_index != SIZE_MAX) {
          road_following.forced_direction_steps--;
          if (road_following.forced_direction_steps == 0) {
            // Check if we found a new segment (will be marked as visited in
            // next frame)
            bool found_new = !road_network->is_visited(next_segment_index);
            if (found_new) {
              road_following.segments_without_reveal = 0; // Reset counter
              // forced_direction_attempts will be reset when segment is marked
              // as visited
            }
          }
        }
      } else {
        // Normal algorithm selection
        if (road_following.current_algorithm == MazeAlgorithm::WallFollower) {
          select_next_wall_follower(road_following, road_network, current_end,
                                    connection_tolerance, next_segment_index,
                                    next_reverse_direction);
        } else if (road_following.current_algorithm == MazeAlgorithm::Tremaux) {
          // TODO: Implement Tremaux
          select_next_wall_follower(road_following, road_network, current_end,
                                    connection_tolerance, next_segment_index,
                                    next_reverse_direction);
        } else if (road_following.current_algorithm == MazeAlgorithm::DFS) {
          // TODO: Implement DFS
          select_next_wall_follower(road_following, road_network, current_end,
                                    connection_tolerance, next_segment_index,
                                    next_reverse_direction);
        } else {
          // AStar - TODO: Implement
          select_next_wall_follower(road_following, road_network, current_end,
                                    connection_tolerance, next_segment_index,
                                    next_reverse_direction);
        }
      }

      if (next_segment_index != SIZE_MAX) {
        second_last_segment_index = last_segment_index;
        last_segment_index = road_following.current_segment_index;
        road_following.current_segment_index = next_segment_index;
        road_following.reverse_direction = next_reverse_direction;
      }
    } else {
      // Continue along current segment
      road_following.progress_along_segment +=
          distance_to_travel / segment_length;
      transform.position.x =
          segment_start.x + road_following.progress_along_segment * direction.x;
      transform.position.y =
          segment_start.y + road_following.progress_along_segment * direction.y;
    }

    road_following.last_position = transform.position;
    transform.velocity.x = normalized_x * road_following.speed;
    transform.velocity.y = normalized_y * road_following.speed;
  }

private:
  static void select_next_wall_follower(RoadFollowing &road_following,
                                        RoadNetwork *road_network,
                                        const vec2 &current_end,
                                        float connection_tolerance,
                                        size_t &next_segment_index,
                                        bool &next_reverse_direction) {
    size_t current_seg = road_following.current_segment_index;
    if (current_seg >= road_network->segment_connections.size()) {
      return;
    }

    // Determine which endpoint we're at: 0 = start, 1 = end
    size_t endpoint_idx = road_following.reverse_direction ? 0 : 1;
    const auto &connections =
        road_network->segment_connections[current_seg][endpoint_idx];

    if (connections.empty()) {
      return;
    }

    // Get current direction for angle calculation
    const RoadSegment &current_segment = road_network->segments[current_seg];
    vec2 current_start = road_following.reverse_direction
                             ? current_segment.end
                             : current_segment.start;
    vec2 current_dir = {current_end.x - current_start.x,
                        current_end.y - current_start.y};
    float current_dir_len = std::sqrt(current_dir.x * current_dir.x +
                                      current_dir.y * current_dir.y);
    if (current_dir_len > 0.001f) {
      current_dir.x /= current_dir_len;
      current_dir.y /= current_dir_len;
    }

    // Collect valid candidates with their angles
    struct Candidate {
      size_t segment_index;
      bool reverse;
      float angle;
      bool is_unvisited;
      float distance;
    };
    std::vector<Candidate> candidates;

    for (const auto &[connected_seg, use_reverse] : connections) {
      if (connected_seg == current_seg) {
        continue;
      }
      if (connected_seg == last_segment_index ||
          connected_seg == second_last_segment_index) {
        continue;
      }

      const RoadSegment &candidate_seg = road_network->segments[connected_seg];
      vec2 cand_start = use_reverse ? candidate_seg.end : candidate_seg.start;
      vec2 cand_end = use_reverse ? candidate_seg.start : candidate_seg.end;

      // Calculate direction vector
      vec2 cand_dir = {cand_end.x - cand_start.x, cand_end.y - cand_start.y};
      float cand_len =
          std::sqrt(cand_dir.x * cand_dir.x + cand_dir.y * cand_dir.y);
      if (cand_len < 0.001f) {
        continue;
      }
      cand_dir.x /= cand_len;
      cand_dir.y /= cand_len;

      // Calculate angle relative to current direction (right-hand rule)
      // Use cross product to determine if right or left
      float cross = current_dir.x * cand_dir.y - current_dir.y * cand_dir.x;
      float dot = current_dir.x * cand_dir.x + current_dir.y * cand_dir.y;
      float angle = std::atan2(cross, dot); // Range: -PI to PI

      // Distance check
      float dist_to_start = std::sqrt(
          (cand_start.x - current_end.x) * (cand_start.x - current_end.x) +
          (cand_start.y - current_end.y) * (cand_start.y - current_end.y));
      float dist_to_end = std::sqrt(
          (cand_end.x - current_end.x) * (cand_end.x - current_end.x) +
          (cand_end.y - current_end.y) * (cand_end.y - current_end.y));

      if (dist_to_start < connection_tolerance ||
          dist_to_end < connection_tolerance) {
        bool is_unvisited = !road_network->is_visited(connected_seg);
        candidates.push_back({connected_seg, use_reverse, angle, is_unvisited,
                              std::min(dist_to_start, dist_to_end)});
      }
    }

    if (candidates.empty()) {
      return;
    }

    // Wall follower: Follow right wall
    // Priority order (for visited segments):
    // 1. Straight ahead (angle closest to 0) - ONLY turn if can't go straight
    // 2. Right turn (positive angle, prefer angles closer to +90°)
    // 3. Left turn (negative angle, prefer angles closer to -90°)
    // 4. Turn around (angle closest to ±180°)
    // Unvisited segments always take priority
    std::sort(
        candidates.begin(), candidates.end(),
        [](const Candidate &a, const Candidate &b) {
          // First priority: unvisited segments
          if (a.is_unvisited != b.is_unvisited) {
            return a.is_unvisited;
          }

          // For visited segments, follow right wall algorithm:
          // Calculate priority scores (lower = higher priority)
          auto get_priority = [](float angle) -> float {
            // Straight (angle ≈ 0): highest priority (only turn if can't go
            // straight)
            float abs_angle = std::abs(angle);
            if (abs_angle < 0.2f) { // ~11 degrees tolerance for "straight"
              return abs_angle;     // 0 is best, small angles are good
            }
            // Right turn (0 to π): second priority
            if (angle > 0.0f) {
              return 1.0f - angle / 3.14159f; // Range: 1.0 (small right) to 0.0
                                              // (90° right)
            }
            // Left turn (-π to 0): third priority
            if (angle < 0.0f) {
              return 2.0f +
                     abs_angle /
                         3.14159f; // Range: 2.0 (small left) to 3.0 (90° left)
            }
            // Turn around (≈ ±π): lowest priority
            return 4.0f;
          };

          float priority_a = get_priority(a.angle);
          float priority_b = get_priority(b.angle);
          return priority_a < priority_b;
        });

    // Select best candidate
    next_segment_index = candidates[0].segment_index;
    next_reverse_direction = candidates[0].reverse;
  }

  static void select_next_forced_direction(RoadFollowing &road_following,
                                           RoadNetwork *road_network,
                                           const vec2 &current_end,
                                           float connection_tolerance,
                                           size_t &next_segment_index,
                                           bool &next_reverse_direction) {
    size_t current_seg = road_following.current_segment_index;
    if (current_seg >= road_network->segment_connections.size()) {
      return;
    }

    size_t endpoint_idx = road_following.reverse_direction ? 0 : 1;
    const auto &connections =
        road_network->segment_connections[current_seg][endpoint_idx];

    if (connections.empty()) {
      return;
    }

    vec2 forced_dir = road_following.forced_direction;
    float best_dot = -1.0f;
    size_t best_seg = SIZE_MAX;
    bool best_reverse = false;

    // First pass: try to find a segment that's not in recent history
    for (const auto &[connected_seg, use_reverse] : connections) {
      if (connected_seg == current_seg) {
        continue;
      }
      // In forced direction mode, prefer segments not in recent history
      // but if that's the only option, we'll allow it in second pass
      if (connected_seg == last_segment_index ||
          connected_seg == second_last_segment_index) {
        continue;
      }

      const RoadSegment &candidate_seg = road_network->segments[connected_seg];
      vec2 cand_start = use_reverse ? candidate_seg.end : candidate_seg.start;
      vec2 cand_end = use_reverse ? candidate_seg.start : candidate_seg.end;

      // Calculate direction vector
      vec2 cand_dir = {cand_end.x - cand_start.x, cand_end.y - cand_start.y};
      float cand_len =
          std::sqrt(cand_dir.x * cand_dir.x + cand_dir.y * cand_dir.y);
      if (cand_len < 0.001f) {
        continue;
      }
      cand_dir.x /= cand_len;
      cand_dir.y /= cand_len;

      // Distance check
      float dist_to_start = std::sqrt(
          (cand_start.x - current_end.x) * (cand_start.x - current_end.x) +
          (cand_start.y - current_end.y) * (cand_start.y - current_end.y));
      float dist_to_end = std::sqrt(
          (cand_end.x - current_end.x) * (cand_end.x - current_end.x) +
          (cand_end.y - current_end.y) * (cand_end.y - current_end.y));

      if (dist_to_start < connection_tolerance ||
          dist_to_end < connection_tolerance) {
        // Calculate dot product with forced direction
        float dot = forced_dir.x * cand_dir.x + forced_dir.y * cand_dir.y;
        if (dot > best_dot) {
          best_dot = dot;
          best_seg = connected_seg;
          best_reverse = use_reverse;
        }
      }
    }

    // If no segment found (all excluded), do a second pass allowing excluded
    // segments This prevents getting stuck when the only connection is in
    // recent history
    if (best_seg == SIZE_MAX) {
      best_dot = -1.0f;
      for (const auto &[connected_seg, use_reverse] : connections) {
        if (connected_seg == current_seg) {
          continue;
        }
        // Now allow even excluded segments if it's the only option
        const RoadSegment &candidate_seg =
            road_network->segments[connected_seg];
        vec2 cand_start = use_reverse ? candidate_seg.end : candidate_seg.start;
        vec2 cand_end = use_reverse ? candidate_seg.start : candidate_seg.end;

        vec2 cand_dir = {cand_end.x - cand_start.x, cand_end.y - cand_start.y};
        float cand_len =
            std::sqrt(cand_dir.x * cand_dir.x + cand_dir.y * cand_dir.y);
        if (cand_len < 0.001f) {
          continue;
        }
        cand_dir.x /= cand_len;
        cand_dir.y /= cand_len;

        float dist_to_start = std::sqrt(
            (cand_start.x - current_end.x) * (cand_start.x - current_end.x) +
            (cand_start.y - current_end.y) * (cand_start.y - current_end.y));
        float dist_to_end = std::sqrt(
            (cand_end.x - current_end.x) * (cand_end.x - current_end.x) +
            (cand_end.y - current_end.y) * (cand_end.y - current_end.y));

        if (dist_to_start < connection_tolerance ||
            dist_to_end < connection_tolerance) {
          float dot = forced_dir.x * cand_dir.x + forced_dir.y * cand_dir.y;
          if (dot > best_dot) {
            best_dot = dot;
            best_seg = connected_seg;
            best_reverse = use_reverse;
          }
        }
      }
    }

    if (best_seg != SIZE_MAX) {
      next_segment_index = best_seg;
      next_reverse_direction = best_reverse;
    }
  }
};

size_t MazeTraversal::last_segment_index = SIZE_MAX;
size_t MazeTraversal::second_last_segment_index = SIZE_MAX;
