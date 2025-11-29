#pragma once

#include "../components.h"
#include "../eq.h"
#include <afterhours/ah.h>

struct RebuildPhotoReveal : afterhours::System<> {
  virtual void once(float) override {
    IsPhotoReveal *photo_reveal =
        afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
    if (!photo_reveal) {
      return;
    }
    if (photo_reveal->merged_rects_dirty) {
      photo_reveal->rebuild_merged_rects();
    }
  }
};
