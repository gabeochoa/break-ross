#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game.h"
#include "../render_backend.h"
#include "../settings.h"
#include <afterhours/ah.h>
#include <string>

struct RenderCurrency : afterhours::System<IsShopManager> {
  virtual void for_each_with(const afterhours::Entity &,
                             const IsShopManager &shop, float) const override {
    int screen_width = Settings::get().get_screen_width();
    int screen_height = Settings::get().get_screen_height();

    float font_size = 20.0f * (static_cast<float>(screen_height) / 720.0f);
    float padding_x = screen_width * 0.02f;
    float padding_y = screen_height * 0.02f;

    std::string text = "Pixels: " + std::to_string(shop.pixels_collected);

    raylib::DrawTextEx(uiFont, text.c_str(), {padding_x, padding_y}, font_size,
                       1.0f, raylib::WHITE);
  }
};
