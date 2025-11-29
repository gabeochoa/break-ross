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

    const float photo_width = 500.0f;
    const float photo_height = 500.0f;
    const float grid_to_photo_x = photo_width / game_constants::GRID_WIDTH;
    const float grid_to_photo_y = photo_height / game_constants::GRID_HEIGHT;

    const float grid_area_width =
        game_constants::GRID_WIDTH * game_constants::BRICK_CELL_SIZE;
    const float grid_area_height =
        game_constants::GRID_HEIGHT * game_constants::BRICK_CELL_SIZE;

    raylib::Rectangle full_source{0.0f, 0.0f, photo_width, -photo_height};
    raylib::Rectangle full_dest{game_constants::BRICK_START_X,
                                game_constants::BRICK_START_Y, grid_area_width,
                                grid_area_height};
    raylib::Color dimmed = {128, 128, 128, 255};
    render_backend::DrawTexturePro(photo_reveal.photo_texture, full_source,
                                   full_dest, {0.0f, 0.0f}, 0.0f, dimmed);

    for (const RevealedRect &rect : photo_reveal.merged_rects) {
      int grid_x = static_cast<int>((rect.x - game_constants::BRICK_START_X) /
                                    game_constants::BRICK_CELL_SIZE);
      int grid_y = static_cast<int>((rect.y - game_constants::BRICK_START_Y) /
                                    game_constants::BRICK_CELL_SIZE);

      int grid_width =
          static_cast<int>(rect.width / game_constants::BRICK_CELL_SIZE);
      int grid_height =
          static_cast<int>(rect.height / game_constants::BRICK_CELL_SIZE);

      float tex_x = grid_x * grid_to_photo_x;
      float tex_y = grid_y * grid_to_photo_y;
      float tex_width = grid_width * grid_to_photo_x;
      float tex_height = grid_height * grid_to_photo_y;

      raylib::Rectangle source{tex_x, tex_y, tex_width, -tex_height};
      raylib::Rectangle dest{rect.x, rect.y, rect.width, rect.height};

      render_backend::BeginScissorMode(
          static_cast<int>(rect.x), static_cast<int>(rect.y),
          static_cast<int>(rect.width), static_cast<int>(rect.height));
      render_backend::DrawTexturePro(photo_reveal.photo_texture, source, dest,
                                     {0.0f, 0.0f}, 0.0f, raylib::WHITE);
      render_backend::EndScissorMode();
    }
  }
};
