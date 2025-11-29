#pragma once

#include <afterhours/ah.h>
#include <afterhours/src/plugins/window_manager.h>

struct RenderFPS : afterhours::System<afterhours::window_manager::ProvidesCurrentResolution> {
  virtual ~RenderFPS() {}
  virtual void for_each_with(
      const afterhours::Entity &,
      const afterhours::window_manager::ProvidesCurrentResolution &pCurrentResolution,
      float) const override {
    raylib::DrawFPS((int)(pCurrentResolution.width() - 80), 0);
  }
};

