#pragma once

#include "../components.h"
#include "../eq.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

struct RenderRoads : afterhours::System<RoadNetwork> {
  virtual void for_each_with(const afterhours::Entity &,
                             const RoadNetwork &road_network,
                             float) const override {
    if (!road_network.is_loaded) {
      return;
    }

    FogOfWar *fog = afterhours::EntityHelper::get_singleton_cmp<FogOfWar>();
    bool check_fog = (fog != nullptr);

    for (const RoadSegment &segment : road_network.segments) {
      if (check_fog) {
        int grid_x1 = game_constants::world_to_grid_x(segment.start.x);
        int grid_y1 = game_constants::world_to_grid_y(segment.start.y);
        int grid_x2 = game_constants::world_to_grid_x(segment.end.x);
        int grid_y2 = game_constants::world_to_grid_y(segment.end.y);

        bool start_revealed = fog->is_revealed(grid_x1, grid_y1);
        bool end_revealed = fog->is_revealed(grid_x2, grid_y2);

        if (!start_revealed && !end_revealed) {
          continue;
        }
      }

      render_backend::DrawLineEx(segment.start, segment.end, segment.width,
                                 raylib::DARKGRAY);
    }
  }
};
