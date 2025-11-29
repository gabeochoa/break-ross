#pragma once

#include "../components.h"
#include "../eq.h"
#include "../log.h"
#include "../log/log_level.h"
#include <afterhours/ah.h>
#include <algorithm>
#include <cmath>
#include <vector>

struct LoopDetection
    : afterhours::System<Transform, RoadFollowing,
                         afterhours::tags::All<ColliderTag::Square>> {
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

    // Check if we're in a loop (too many consecutive visited segments)
    if (road_following.segments_without_reveal >=
        RoadFollowing::LOOP_DETECTION_THRESHOLD) {
      // Enter forced direction mode if not already in it
      if (road_following.forced_direction_steps == 0) {
        road_following.forced_direction_attempts++;

        // If we've tried forced direction multiple times and still stuck,
        // jump to a random unvisited segment
        if (road_following.forced_direction_attempts >=
            RoadFollowing::MAX_FORCED_DIRECTION_ATTEMPTS) {
          size_t unvisited_seg = road_network->find_random_unvisited_segment();
          if (unvisited_seg != SIZE_MAX) {
            road_following.current_segment_index = unvisited_seg;
            road_following.progress_along_segment = 0.0f;
            road_following.reverse_direction = false;
            road_following.segments_without_reveal = 0;
            road_following.forced_direction_attempts = 0;
            // Update transform position to the start of the new segment
            const RoadSegment &new_seg = road_network->segments[unvisited_seg];
            transform.position = new_seg.start;
            log_info("LoopDetection: Jumping to unvisited segment {} after {} "
                     "forced direction attempts",
                     unvisited_seg, road_following.forced_direction_attempts);
            return;
          } else {
            // All segments visited! Reset attempts counter
            road_following.forced_direction_attempts = 0;
            log_info("LoopDetection: All segments visited!");
          }
        }

        // Calculate current direction
        const RoadSegment &current_seg =
            road_network->segments[road_following.current_segment_index];
        vec2 segment_start = road_following.reverse_direction
                                 ? current_seg.end
                                 : current_seg.start;
        vec2 segment_end = road_following.reverse_direction ? current_seg.start
                                                            : current_seg.end;
        vec2 current_dir = {segment_end.x - segment_start.x,
                            segment_end.y - segment_start.y};
        float dir_len = std::sqrt(current_dir.x * current_dir.x +
                                  current_dir.y * current_dir.y);
        if (dir_len > 0.001f) {
          // Rotate current direction 90 degrees to the right to break the loop
          // This ensures we're not going back into the loop
          float normalized_x = current_dir.x / dir_len;
          float normalized_y = current_dir.y / dir_len;
          float rotated_x = -normalized_y; // 90 degree rotation
          float rotated_y = normalized_x;
          road_following.forced_direction.x = rotated_x;
          road_following.forced_direction.y = rotated_y;
        } else {
          road_following.forced_direction = {1.0f, 0.0f}; // Default to right
        }
        road_following.forced_direction_steps =
            RoadFollowing::FORCED_DIRECTION_STEPS;
        log_info("LoopDetection: Loop detected! (attempt {}/{}) Forcing "
                 "direction ({:.2f}, "
                 "{:.2f}) for {} steps",
                 road_following.forced_direction_attempts,
                 RoadFollowing::MAX_FORCED_DIRECTION_ATTEMPTS,
                 road_following.forced_direction.x,
                 road_following.forced_direction.y,
                 road_following.forced_direction_steps);
      }
    }
    // Note: Don't reset attempts counter here - it should only reset when we
    // actually reveal a NEW segment (handled in MazeTraversal when
    // just_revealed is true)

    // If in forced direction mode, override segment selection
    if (road_following.forced_direction_steps > 0) {
      // This will be handled in MazeTraversal by checking
      // forced_direction_steps For now, just decrement the counter
      road_following.forced_direction_timer += dt;
      // Decrement steps when we reach a segment end (handled in MazeTraversal)
    }
  }
};
