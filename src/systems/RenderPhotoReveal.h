#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

struct RenderPhotoReveal : afterhours::System<IsPhotoReveal> {
  virtual void for_each_with(const afterhours::Entity &,
                             const IsPhotoReveal &photo_reveal,
                             float) const override {
    if (!photo_reveal.is_loaded) {
      return;
    }
    if (photo_reveal.revealed_cells.none()) {
      return;
    }

    photo_reveal.update_mask_texture();

    const float photo_width = 500.0f;
    const float photo_height = 500.0f;
    const float grid_area_width =
        game_constants::GRID_WIDTH * game_constants::BRICK_CELL_SIZE;
    const float grid_area_height =
        game_constants::GRID_HEIGHT * game_constants::BRICK_CELL_SIZE;

    raylib::Rectangle full_source{0.0f, 0.0f, photo_width, -photo_height};
    raylib::Rectangle full_dest{game_constants::BRICK_START_X,
                                game_constants::BRICK_START_Y, grid_area_width,
                                grid_area_height};

    for (const RevealedRect &rect : photo_reveal.merged_rects) {
      render_backend::BeginScissorMode(
          static_cast<int>(rect.x), static_cast<int>(rect.y),
          static_cast<int>(rect.width), static_cast<int>(rect.height));
      render_backend::DrawTexturePro(photo_reveal.photo_texture, full_source,
                                     full_dest, {0.0f, 0.0f}, 0.0f,
                                     raylib::WHITE);
      render_backend::EndScissorMode();
    }
  }
};
