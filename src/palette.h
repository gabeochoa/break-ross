#pragma once

#include "rl.h"

// Micrographic / technical-blueprint palette (see last_mile DESIGN.md):
// near-black ground, bone "ink" for lines and text, one hot accent for the
// player, cool blue reserved for rivals.
namespace palette {
inline constexpr raylib::Color BG{15, 15, 15, 255};       // #0F0F0F ground
inline constexpr raylib::Color INK{236, 231, 218, 255};   // #ECE7DA lines/text
inline constexpr raylib::Color INK_DIM{236, 231, 218, 90}; // faint ink (unmapped)
inline constexpr raylib::Color ACCENT{232, 84, 30, 255};  // #E8541E player
inline constexpr raylib::Color RIVAL{76, 134, 232, 255};  // #4C86E8 rivals
inline constexpr raylib::Color PANEL{20, 20, 20, 240};    // shop/HUD panel fill
} // namespace palette
