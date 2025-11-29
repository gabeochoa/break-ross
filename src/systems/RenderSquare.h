#pragma once

#include "../components.h"
#include "../eq.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

struct RenderSquare
    : afterhours::System<Transform, RoadFollowing,
                         afterhours::tags::All<ColliderTag::Square>> {
  virtual void for_each_with(const afterhours::Entity &,
                             const Transform &transform,
                             const RoadFollowing &road_following,
                             float) const override {
    vec2 center_pos = {transform.position.x - transform.size.x * 0.5f,
                       transform.position.y - transform.size.y * 0.5f};

    // Color based on algorithm or forced direction mode
    raylib::Color square_color = raylib::GREEN;
    if (road_following.forced_direction_steps > 0) {
      square_color = raylib::ORANGE; // Orange when breaking loop
    } else if (road_following.current_algorithm == MazeAlgorithm::WallFollower) {
      square_color = raylib::GREEN;
    } else if (road_following.current_algorithm == MazeAlgorithm::Tremaux) {
      square_color = raylib::BLUE;
    } else if (road_following.current_algorithm == MazeAlgorithm::DFS) {
      square_color = raylib::PURPLE;
    } else {
      square_color = raylib::YELLOW; // AStar
    }

    render_backend::DrawRectangleV(center_pos, transform.size, square_color);

    vec2 corner = {transform.position.x - 2.0f, transform.position.y - 2.0f};
    render_backend::DrawRectangleV(corner, vec2{4.0f, 4.0f}, raylib::RED);
  }
};
