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

inline void SetTextureFilter(raylib::Texture2D texture, int filter) {
  raylib::SetTextureFilter(texture, filter);
}

inline raylib::Shader LoadShader(const char *vsFileName,
                                 const char *fsFileName) {
  return raylib::LoadShader(vsFileName, fsFileName);
}

inline void UnloadShader(raylib::Shader shader) {
  raylib::UnloadShader(shader);
}

inline int GetShaderLocation(raylib::Shader shader, const char *uniformName) {
  return raylib::GetShaderLocation(shader, uniformName);
}

inline void SetShaderValue(raylib::Shader shader, int locIndex,
                           const void *value, int uniformType) {
  raylib::SetShaderValue(shader, locIndex, value, uniformType);
}

inline void SetShaderValueTexture(raylib::Shader shader, int locIndex,
                                  raylib::Texture2D texture) {
  raylib::SetShaderValueTexture(shader, locIndex, texture);
}

inline void BeginShaderMode(raylib::Shader shader) {
  raylib::BeginShaderMode(shader);
}

inline void EndShaderMode() { raylib::EndShaderMode(); }

inline raylib::Image GenImageColor(int width, int height, raylib::Color color) {
  return raylib::GenImageColor(width, height, color);
}

inline raylib::Texture2D LoadTextureFromImage(raylib::Image image) {
  return raylib::LoadTextureFromImage(image);
}

inline void UpdateTexture(raylib::Texture2D texture, const void *pixels) {
  raylib::UpdateTexture(texture, pixels);
}

inline void UnloadImage(raylib::Image image) { raylib::UnloadImage(image); }
} // namespace render_backend
