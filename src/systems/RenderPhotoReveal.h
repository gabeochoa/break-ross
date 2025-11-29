#pragma once

#include "../components.h"
#include "../eq.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

struct RenderPhotoReveal : afterhours::System<IsPhotoReveal> {
  virtual void for_each_with(const afterhours::Entity &,
                             const IsPhotoReveal &photo_reveal,
                             float) const override {
    for (const RevealedRect &rect : photo_reveal.merged_rects) {
      render_backend::DrawRectangleRec(
          {rect.x, rect.y, rect.width, rect.height}, raylib::DARKGREEN);
    }
  }
};
