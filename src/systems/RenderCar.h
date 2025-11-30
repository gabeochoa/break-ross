#pragma once

#include "../components.h"
#include "../eq.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

struct RenderCar
    : afterhours::System<Transform, RoadFollowing,
                         afterhours::tags::All<ColliderTag::Circle>> {
  virtual void for_each_with(const afterhours::Entity &,
                             const Transform &transform,
                             const RoadFollowing &road_following,
                             float) const override {
    raylib::Color car_color = raylib::GREEN;
    if (road_following.forced_direction_steps > 0) {
      car_color = raylib::ORANGE;
    } else if (road_following.current_algorithm ==
               MazeAlgorithm::WallFollower) {
      car_color = raylib::GREEN;
    } else if (road_following.current_algorithm == MazeAlgorithm::Tremaux) {
      car_color = raylib::BLUE;
    } else if (road_following.current_algorithm == MazeAlgorithm::DFS) {
      car_color = raylib::PURPLE;
    } else {
      car_color = raylib::YELLOW;
    }

    render_backend::DrawCircleV(transform.position, transform.size.x / 2.0f,
                                car_color);
  }
};
