#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../log.h"
#include "../render_backend.h"
#include <afterhours/ah.h>
#include <cmath>

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
    if (photo_reveal.photo_texture.id == 0) {
      return;
    }

    photo_reveal.update_mask_texture();

    float tex_width = static_cast<float>(photo_reveal.photo_texture.width);
    float tex_height = static_cast<float>(photo_reveal.photo_texture.height);

    render_backend::SetTextureFilter(photo_reveal.photo_texture,
                                     raylib::TEXTURE_FILTER_POINT);

    // Use merged rectangles with stored grid coordinates for perfect 1:1 mapping
    // merged_rects should already be built by RebuildPhotoReveal system
    
    float pixels_per_grid_cell_x = tex_width / static_cast<float>(game_constants::GRID_WIDTH);
    float pixels_per_grid_cell_y = tex_height / static_cast<float>(game_constants::GRID_HEIGHT);

    for (const RevealedRect &rect : photo_reveal.merged_rects) {
      // Use stored grid coordinates directly - no conversion needed
      float source_x = static_cast<float>(rect.grid_x) * pixels_per_grid_cell_x;
      float source_y = static_cast<float>(rect.grid_y) * pixels_per_grid_cell_y;
      float source_w = static_cast<float>(rect.grid_width) * pixels_per_grid_cell_x;
      float source_h = static_cast<float>(rect.grid_height) * pixels_per_grid_cell_y;
      
      raylib::Rectangle source_rect{
          source_x,
          source_y,
          source_w,
          -source_h};

      raylib::Rectangle dest_rect{rect.x, rect.y, rect.width, rect.height};

      render_backend::DrawTexturePro(photo_reveal.photo_texture, source_rect,
                                    dest_rect, {0.0f, 0.0f}, 0.0f,
                                    raylib::WHITE);
    }
  }
};
