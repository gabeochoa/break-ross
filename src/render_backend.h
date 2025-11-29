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

inline void DrawTexture(raylib::Texture2D texture, int posX, int posY,
                        raylib::Color tint) {
  raylib::DrawTexture(texture, posX, posY, tint);
}

inline void DrawTextureRec(raylib::Texture2D texture, raylib::Rectangle source,
                           raylib::Vector2 position, raylib::Color tint) {
  raylib::DrawTextureRec(texture, source, position, tint);
}

inline void DrawTexturePro(raylib::Texture2D texture, raylib::Rectangle source,
                           raylib::Rectangle dest, raylib::Vector2 origin,
                           float rotation, raylib::Color tint) {
  raylib::DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

inline raylib::Texture2D LoadTexture(const char *fileName) {
  return raylib::LoadTexture(fileName);
}

inline void UnloadTexture(raylib::Texture2D texture) {
  raylib::UnloadTexture(texture);
}

inline void BeginScissorMode(int x, int y, int width, int height) {
  raylib::BeginScissorMode(x, y, width, height);
}

inline void EndScissorMode() { raylib::EndScissorMode(); }
} // namespace render_backend
