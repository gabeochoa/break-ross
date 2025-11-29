#pragma once

#include "rl.h"

namespace render_backend {
inline void BeginDrawing() { raylib::BeginDrawing(); }

inline void EndDrawing() { raylib::EndDrawing(); }

inline void BeginTextureMode(raylib::RenderTexture2D target) {
  raylib::BeginTextureMode(target);
}

inline void EndTextureMode() { raylib::EndTextureMode(); }

inline void ClearBackground(raylib::Color color) {
  raylib::ClearBackground(color);
}

inline void DrawCircleV(vec2 center, float radius, raylib::Color color) {
  raylib::DrawCircleV(center, radius, color);
}

inline void DrawRectangleRec(raylib::Rectangle rec, raylib::Color color) {
  raylib::DrawRectangleRec(rec, color);
}
} // namespace render_backend
