#pragma once

#include "../components.h"
#include "../eq.h"
#include "../render_backend.h"
#include "../rl.h"
#include <afterhours/ah.h>

struct RenderPOIs : afterhours::System<PointOfInterest> {
  virtual void for_each_with(const afterhours::Entity &,
                             const PointOfInterest &poi, float) const override {
    if (!poi.is_discovered) {
      return;
    }

    IsPhotoReveal *photo_reveal =
        afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
    invariant(photo_reveal, "IsPhotoReveal singleton not found");
    int grid_x = game_constants::world_to_grid_x(poi.position.x);
    int grid_y = game_constants::world_to_grid_y(poi.position.y);

    if (!photo_reveal->is_revealed(grid_x, grid_y)) {
      return;
    }

    raylib::Color poi_color = get_poi_color(poi.poi_type);
    float poi_size = get_poi_size(poi.poi_type);

    render_backend::DrawCircleV(poi.position, poi_size, poi_color);
    raylib::DrawCircleLines(static_cast<int>(poi.position.x),
                            static_cast<int>(poi.position.y),
                            static_cast<int>(poi_size), raylib::WHITE);
  }

private:
  raylib::Color get_poi_color(POIType poi_type) const {
    switch (poi_type) {
    case POIType::Landmark:
      return raylib::YELLOW;
    case POIType::City:
      return raylib::ORANGE;
    case POIType::Area:
    default:
      return raylib::BLUE;
    }
  }

  float get_poi_size(POIType poi_type) const {
    switch (poi_type) {
    case POIType::Landmark:
      return 8.0f;
    case POIType::City:
      return 6.0f;
    case POIType::Area:
    default:
      return 4.0f;
    }
  }
};
