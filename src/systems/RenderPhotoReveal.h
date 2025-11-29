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

    afterhours::camera::HasCamera *camera =
        afterhours::EntityHelper::get_singleton_cmp<
            afterhours::camera::HasCamera>();

    float viewport_min_x = 0.0f;
    float viewport_min_y = 0.0f;
    float viewport_max_x = game_constants::WORLD_WIDTH;
    float viewport_max_y = game_constants::WORLD_HEIGHT;

    if (camera) {
      float screen_w = static_cast<float>(raylib::GetScreenWidth());
      float screen_h = static_cast<float>(raylib::GetScreenHeight());

      vec2 screen_top_left =
          raylib::GetScreenToWorld2D({0.0f, 0.0f}, camera->camera);
      vec2 screen_bottom_right =
          raylib::GetScreenToWorld2D({screen_w, screen_h}, camera->camera);

      viewport_min_x =
          std::min(screen_top_left.x, screen_bottom_right.x) - 100.0f;
      viewport_min_y =
          std::min(screen_top_left.y, screen_bottom_right.y) - 100.0f;
      viewport_max_x =
          std::max(screen_top_left.x, screen_bottom_right.x) + 100.0f;
      viewport_max_y =
          std::max(screen_top_left.y, screen_bottom_right.y) + 100.0f;
    }

    int rendered_count = 0;
    constexpr int MAX_RECTS_PER_FRAME = 200;

    for (const RevealedRect &rect : photo_reveal.merged_rects) {
      if (rendered_count >= MAX_RECTS_PER_FRAME) {
        break;
      }

      float rect_max_x = rect.x + rect.width;
      float rect_max_y = rect.y + rect.height;

      if (rect_max_x < viewport_min_x || rect.x > viewport_max_x ||
          rect_max_y < viewport_min_y || rect.y > viewport_max_y) {
        continue;
      }

      render_backend::BeginScissorMode(
          static_cast<int>(rect.x), static_cast<int>(rect.y),
          static_cast<int>(rect.width), static_cast<int>(rect.height));
      render_backend::DrawTexturePro(photo_reveal.photo_texture, full_source,
                                     full_dest, {0.0f, 0.0f}, 0.0f,
                                     raylib::WHITE);
      render_backend::EndScissorMode();
      rendered_count++;
    }
  }
};
