#pragma once

#include "../components.h"
#include "../game.h"
#include "../render_backend.h"
#include <afterhours/ah.h>

struct BeginWorldRender : afterhours::System<> {
  virtual void once(float) const override {
    render_backend::BeginTextureMode(mainRT);
    render_backend::ClearBackground(raylib::BLACK);
  }
};

struct EndWorldRender : afterhours::System<> {
  virtual void once(float) const override { render_backend::EndTextureMode(); }
};

struct BeginPostProcessingRender : afterhours::System<> {
  virtual void once(float) const override {
    render_backend::BeginDrawing();
    render_backend::ClearBackground(raylib::BLACK);
  }
};

struct EndDrawing : afterhours::System<> {
  virtual void once(float) const override { render_backend::EndDrawing(); }
};
