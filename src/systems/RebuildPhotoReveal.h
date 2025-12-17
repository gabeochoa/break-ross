#pragma once

#include "../components.h"
#include "../eq.h"
#include <afterhours/ah.h>

struct RebuildPhotoReveal : afterhours::System<IsPhotoReveal> {
  virtual void for_each_with(afterhours::Entity &, IsPhotoReveal &photo_reveal,
                             float) override {
    if (!photo_reveal.merged_rects_dirty) {
      return;
    }
    photo_reveal.rebuild_merged_rects();
  }
};
