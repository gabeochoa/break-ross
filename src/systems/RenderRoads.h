#pragma once

#include "../components.h"
#include "../eq.h"
#include "../palette.h"
#include "../render_backend.h"
#include "MapRevealSystem.h"
#include <afterhours/ah.h>

struct RenderRoads : afterhours::System<RoadNetwork> {
  virtual void for_each_with(const afterhours::Entity &,
                             const RoadNetwork &road_network,
                             float) const override {
    if (!road_network.is_loaded) {
      return;
    }

    IsPhotoReveal *photo_reveal =
        afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
    invariant(photo_reveal, "IsPhotoReveal singleton not found");

    for (size_t i = 0; i < road_network.segments.size(); ++i) {
      const RoadSegment &segment = road_network.segments[i];

      int grid_x1 = game_constants::world_to_grid_x(segment.start.x);
      int grid_y1 = game_constants::world_to_grid_y(segment.start.y);
      int grid_x2 = game_constants::world_to_grid_x(segment.end.x);
      int grid_y2 = game_constants::world_to_grid_y(segment.end.y);

      bool start_revealed = photo_reveal->is_revealed(grid_x1, grid_y1);
      bool end_revealed = photo_reveal->is_revealed(grid_x2, grid_y2);

      if (!start_revealed && !end_revealed) {
        continue;
      }

      bool is_mapped = MapRevealSystem::query_segment(i);
      raylib::Color road_color = get_road_color(segment.road_type, is_mapped);
      float road_width = get_road_width(segment.road_type);

      render_backend::DrawLineEx(segment.start, segment.end, road_width,
                                 road_color);
    }
  }

private:
  raylib::Color get_road_color(RoadType /* road_type */, bool is_mapped) const {
    // Blueprint style: streets revealed-but-not-yet-mapped are faint ink;
    // mapped streets light up in the accent color.
    return is_mapped ? palette::ACCENT : palette::INK_DIM;
  }

  float get_road_width(RoadType road_type) const {
    switch (road_type) {
    case RoadType::Highway:
      return 5.0f;
    case RoadType::Primary:
      return 4.0f;
    case RoadType::Secondary:
      return 3.0f;
    case RoadType::Residential:
    default:
      return 2.0f;
    }
  }
};
