#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../log.h"
#include "MapRevealSystem.h"
#include <afterhours/ah.h>
#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

struct MazeTraversal
    : afterhours::System<
          Transform, RoadFollowing,
          afterhours::tags::Any<ColliderTag::Square, ColliderTag::Circle>> {
  static size_t last_segment_index;
  static size_t second_last_segment_index;

  static bool detect_loop(const std::vector<size_t> &recent_segments,
                          size_t next_seg) {
    if (recent_segments.size() < 4) {
      return false;
    }

    for (size_t pattern_len = 2; pattern_len <= recent_segments.size() / 2;
         ++pattern_len) {
      bool is_loop = true;
      for (size_t i = 0; i < pattern_len; ++i) {
        size_t idx1 = recent_segments.size() - i;
        size_t idx2 = recent_segments.size() - pattern_len - i;
        size_t seg1 = (i == 0) ? next_seg : recent_segments[idx1 - 1];
        if (idx2 >= recent_segments.size() || seg1 != recent_segments[idx2]) {
          is_loop = false;
          break;
        }
      }
      if (is_loop && pattern_len >= 3) {
        return true;
      }
    }

    return false;
  }

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

    bool just_revealed =
        MapRevealSystem::reveal_segment(road_following.current_segment_index);

    FogOfWar *fog = afterhours::EntityHelper::get_singleton_cmp<FogOfWar>();
    invariant(fog, "FogOfWar singleton not found");
    float reveal_percentage = fog->get_reveal_percentage();
    bool prioritize_unvisited = reveal_percentage >= 90.0f;

    // Update segments_without_reveal counter
    if (just_revealed) {
      road_following.segments_without_reveal = 0;
      road_following.forced_direction_attempts =
          0; // Reset when we reveal a new segment
    } else {
      road_following.segments_without_reveal++;
    }

    // At 90%+, if we've been going through visited segments for too long, jump
    // to unvisited
    if (prioritize_unvisited && road_following.segments_without_reveal >= 5) {
      size_t random_unvisited = road_network->find_random_unvisited_segment();
      if (random_unvisited != SIZE_MAX) {
        log_info("MazeTraversal: At 90%+, jumping to unvisited segment {} "
                 "after {} visited segments",
                 random_unvisited, road_following.segments_without_reveal);
        road_following.current_segment_index = random_unvisited;
        road_following.progress_along_segment = 0.0f;
        road_following.reverse_direction = false;
        road_following.segments_without_reveal = 0;
        road_following.segment_history.clear();
        const RoadSegment &new_seg = road_network->segments[random_unvisited];
        transform.position = new_seg.start;
        return;
      }
    }

    // Move along current segment
    float distance_to_travel = road_following.speed * dt;
    float remaining_on_segment =
        (1.0f - road_following.progress_along_segment) * segment_length;

    if (distance_to_travel < remaining_on_segment) {
      // Continue along current segment
      road_following.progress_along_segment +=
          distance_to_travel / segment_length;
      transform.position.x =
          segment_start.x + road_following.progress_along_segment * direction.x;
      transform.position.y =
          segment_start.y + road_following.progress_along_segment * direction.y;

      road_following.last_position = transform.position;
      transform.velocity.x = normalized_x * road_following.speed;
      transform.velocity.y = normalized_y * road_following.speed;
      return;
    }

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
      // Normal algorithm selection - all algorithms currently use wall follower
      select_next_wall_follower(road_following, road_network, current_end,
                                connection_tolerance, next_segment_index,
                                next_reverse_direction);

      // At 90%+, if no unvisited segments found at junction, jump to one
      if (prioritize_unvisited && next_segment_index != SIZE_MAX) {
        bool next_is_unvisited = !road_network->is_visited(next_segment_index);
        if (!next_is_unvisited) {
          size_t random_unvisited =
              road_network->find_random_unvisited_segment();
          if (random_unvisited != SIZE_MAX) {
            log_info("MazeTraversal: At 90%+, no unvisited at junction, "
                     "jumping to unvisited segment {}",
                     random_unvisited);
            next_segment_index = random_unvisited;
            next_reverse_direction = false;
            road_following.segment_history.clear();
          }
        }
      }
    }

    if (next_segment_index != SIZE_MAX) {
      road_following.segment_history.push_back(
          road_following.current_segment_index);
      if (road_following.segment_history.size() >
          RoadFollowing::MAX_HISTORY_SIZE) {
        road_following.segment_history.erase(
            road_following.segment_history.begin());
      }

      std::vector<size_t> test_history = road_following.segment_history;
      test_history.push_back(next_segment_index);
      bool is_loop = detect_loop(test_history, next_segment_index);

      if (is_loop) {
        size_t random_unvisited = road_network->find_random_unvisited_segment();
        if (random_unvisited != SIZE_MAX) {
          log_warn(
              "MazeTraversal: Loop detected, jumping to unvisited segment {}",
              random_unvisited);
          next_segment_index = random_unvisited;
          next_reverse_direction = false;
          road_following.segment_history.clear();
        } else {
          log_warn("MazeTraversal: Loop detected but no unvisited segments "
                   "available");
        }
      }

      second_last_segment_index = last_segment_index;
      last_segment_index = road_following.current_segment_index;
      road_following.current_segment_index = next_segment_index;
      road_following.reverse_direction = next_reverse_direction;
    } else {
      log_warn("MazeTraversal: Car stuck at segment {}, no next segment found",
               road_following.current_segment_index);
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

    FogOfWar *fog = afterhours::EntityHelper::get_singleton_cmp<FogOfWar>();
    invariant(fog, "FogOfWar singleton not found");
    float reveal_percentage = fog->get_reveal_percentage();
    bool prioritize_unvisited = reveal_percentage >= 90.0f;

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
    if (current_dir_len <= 0.001f) {
      return;
    }
    current_dir.x /= current_dir_len;
    current_dir.y /= current_dir_len;

    // Collect valid candidates with their angles
    struct Candidate {
      size_t segment_index;
      bool reverse;
      float angle;
      bool is_unvisited;
      float distance;
    };
    std::vector<Candidate> candidates;

    std::set<size_t> recent_segments;
    recent_segments.insert(last_segment_index);
    recent_segments.insert(second_last_segment_index);
    for (size_t hist_seg : road_following.segment_history) {
      recent_segments.insert(hist_seg);
    }

    for (const auto &[connected_seg, use_reverse] : connections) {
      if (connected_seg == current_seg) {
        continue;
      }
      if (recent_segments.find(connected_seg) != recent_segments.end()) {
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

      if (dist_to_start >= connection_tolerance &&
          dist_to_end >= connection_tolerance) {
        continue;
      }
      bool is_unvisited = !road_network->is_visited(connected_seg);
      candidates.push_back({connected_seg, use_reverse, angle, is_unvisited,
                            std::min(dist_to_start, dist_to_end)});
    }

    if (candidates.empty()) {
      std::set<size_t> recent_segments_fallback;
      recent_segments_fallback.insert(last_segment_index);
      recent_segments_fallback.insert(second_last_segment_index);
      for (const auto &[connected_seg, use_reverse] : connections) {
        if (connected_seg == current_seg) {
          continue;
        }
        if (recent_segments_fallback.find(connected_seg) !=
            recent_segments_fallback.end()) {
          continue;
        }
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
        float cross = current_dir.x * cand_dir.y - current_dir.y * cand_dir.x;
        float dot = current_dir.x * cand_dir.x + current_dir.y * cand_dir.y;
        float angle = std::atan2(cross, dot);
        float dist_to_start = std::sqrt(
            (cand_start.x - current_end.x) * (cand_start.x - current_end.x) +
            (cand_start.y - current_end.y) * (cand_start.y - current_end.y));
        float dist_to_end = std::sqrt(
            (cand_end.x - current_end.x) * (cand_end.x - current_end.x) +
            (cand_end.y - current_end.y) * (cand_end.y - current_end.y));
        if (dist_to_start >= connection_tolerance &&
            dist_to_end >= connection_tolerance) {
          continue;
        }
        bool is_unvisited = !road_network->is_visited(connected_seg);
        candidates.push_back({connected_seg, use_reverse, angle, is_unvisited,
                              std::min(dist_to_start, dist_to_end)});
      }
      if (candidates.empty()) {
        return;
      }
    }

    if (prioritize_unvisited) {
      std::vector<Candidate> unvisited_candidates;
      std::vector<Candidate> visited_candidates;
      for (const auto &cand : candidates) {
        if (cand.is_unvisited) {
          unvisited_candidates.push_back(cand);
        } else {
          visited_candidates.push_back(cand);
        }
      }

      if (!unvisited_candidates.empty()) {
        std::sort(unvisited_candidates.begin(), unvisited_candidates.end(),
                  [](const Candidate &a, const Candidate &b) {
                    float abs_angle_a = std::abs(a.angle);
                    float abs_angle_b = std::abs(b.angle);
                    if (abs_angle_a < 0.1f && abs_angle_b >= 0.1f) {
                      return true;
                    }
                    if (abs_angle_a >= 0.1f && abs_angle_b < 0.1f) {
                      return false;
                    }
                    if (abs_angle_a < 0.1f && abs_angle_b < 0.1f) {
                      return abs_angle_a < abs_angle_b;
                    }
                    if (a.angle > 0.0f && b.angle <= 0.0f) {
                      return true;
                    }
                    if (a.angle <= 0.0f && b.angle > 0.0f) {
                      return false;
                    }
                    return abs_angle_a < abs_angle_b;
                  });
        next_segment_index = unvisited_candidates[0].segment_index;
        next_reverse_direction = unvisited_candidates[0].reverse;
        return;
      }
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
            if (abs_angle <
                0.1f) { // ~6 degrees tolerance for "straight" - stricter
              return abs_angle; // 0 is best, small angles are good
            }
            // Right turn (0 to π): second priority
            if (angle > 0.0f) {
              return 1.0f + angle / 3.14159f; // Range: 1.0 (small right) to 2.0
                                              // (90° right)
            }
            // Left turn (-π to 0): third priority
            if (angle < 0.0f) {
              return 3.0f +
                     abs_angle /
                         3.14159f; // Range: 3.0 (small left) to 4.0 (90° left)
            }
            // Turn around (≈ ±π): lowest priority
            return 5.0f;
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

    std::set<size_t> recent_segments;
    recent_segments.insert(last_segment_index);
    recent_segments.insert(second_last_segment_index);
    for (size_t hist_seg : road_following.segment_history) {
      recent_segments.insert(hist_seg);
    }

    // First pass: try to find a segment that's not in recent history
    for (const auto &[connected_seg, use_reverse] : connections) {
      if (connected_seg == current_seg) {
        continue;
      }
      // In forced direction mode, prefer segments not in recent history
      // but if that's the only option, we'll allow it in second pass
      if (recent_segments.find(connected_seg) != recent_segments.end()) {
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

      if (dist_to_start >= connection_tolerance &&
          dist_to_end >= connection_tolerance) {
        continue;
      }
      // Calculate dot product with forced direction
      float dot = forced_dir.x * cand_dir.x + forced_dir.y * cand_dir.y;
      if (dot <= best_dot) {
        continue;
      }
      best_dot = dot;
      best_seg = connected_seg;
      best_reverse = use_reverse;
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

        if (dist_to_start >= connection_tolerance &&
            dist_to_end >= connection_tolerance) {
          continue;
        }
        float dot = forced_dir.x * cand_dir.x + forced_dir.y * cand_dir.y;
        if (dot <= best_dot) {
          continue;
        }
        best_dot = dot;
        best_seg = connected_seg;
        best_reverse = use_reverse;
      }
    }

    if (best_seg == SIZE_MAX) {
      return;
    }
    next_segment_index = best_seg;
    next_reverse_direction = best_reverse;
  }
};

size_t MazeTraversal::last_segment_index = SIZE_MAX;
size_t MazeTraversal::second_last_segment_index = SIZE_MAX;
