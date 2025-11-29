#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game.h"
#include "../render_backend.h"
#include "../settings.h"
#include <afterhours/ah.h>
#include <string>

struct RenderGameUI : afterhours::System<IsShopManager> {
  virtual void for_each_with(const afterhours::Entity &,
                             const IsShopManager &shop_const,
                             float) const override {
    IsShopManager *shop = const_cast<IsShopManager *>(&shop_const);
    int screen_width = Settings::get().get_screen_width();
    int screen_height = Settings::get().get_screen_height();

    float font_size = 20.0f * (static_cast<float>(screen_height) / 720.0f);
    float padding_x = screen_width * 0.02f;
    float padding_y = screen_height * 0.02f;
    float line_spacing = font_size * 1.2f;

    render_pixels_text(shop, padding_x, padding_y, font_size);
    render_photo_reveal(padding_x, padding_y, line_spacing, font_size);

    if (shop->shop_open) {
      render_shop(shop, screen_width, screen_height, font_size, line_spacing);
    } else {
      render_shop_hint(padding_x, padding_y, line_spacing, font_size);
    }
  }

private:
  void render_pixels_text(IsShopManager *shop, float x, float y,
                          float font_size) const {
    std::string pixels_text =
        "Pixels: " + std::to_string(shop->pixels_collected);
    raylib::DrawTextEx(uiFont, pixels_text.c_str(), {x, y}, font_size, 1.0f,
                       raylib::WHITE);
  }

  void render_photo_reveal(float padding_x, float padding_y, float line_spacing,
                           float font_size) const {
    IsPhotoReveal *photo_reveal =
        afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
    if (photo_reveal) {
      photo_reveal->update_reveal_percentage();
      std::string reveal_text =
          "Revealed: " +
          std::to_string(static_cast<int>(photo_reveal->reveal_percentage)) +
          "%";
      raylib::DrawTextEx(uiFont, reveal_text.c_str(),
                         {padding_x, padding_y + line_spacing}, font_size, 1.0f,
                         raylib::WHITE);
    }
  }

  void render_shop(IsShopManager *shop, int screen_width, int screen_height,
                   float font_size, float line_spacing) const {
    float shop_x = screen_width * 0.5f;
    float shop_y = screen_height * 0.5f;
    float shop_width = screen_width * 0.4f;
    float shop_height = screen_height * 0.6f;
    float shop_padding = 20.0f;

    raylib::DrawRectangle(static_cast<int>(shop_x - shop_width / 2.0f),
                          static_cast<int>(shop_y - shop_height / 2.0f),
                          static_cast<int>(shop_width),
                          static_cast<int>(shop_height),
                          raylib::Color{50, 50, 50, 240});

    raylib::DrawRectangleLines(static_cast<int>(shop_x - shop_width / 2.0f),
                               static_cast<int>(shop_y - shop_height / 2.0f),
                               static_cast<int>(shop_width),
                               static_cast<int>(shop_height), raylib::WHITE);

    float shop_title_y = shop_y - shop_height / 2.0f + shop_padding;
    raylib::DrawTextEx(
        uiFont, "SHOP (TAB to close)",
        {shop_x - shop_width / 2.0f + shop_padding, shop_title_y},
        font_size * 1.5f, 1.0f, raylib::WHITE);

    float item_y = shop_title_y + line_spacing * 2.0f;
    float item_spacing = line_spacing * 1.5f;
    float button_height = line_spacing * 1.2f;
    float button_width = shop_width - shop_padding * 2.0f;

    raylib::Vector2 mouse_pos = raylib::GetMousePosition();
    bool mouse_clicked =
        raylib::IsMouseButtonPressed(raylib::MOUSE_BUTTON_LEFT);

    item_y = render_shop_button(
        shop, shop_x, shop_width, shop_padding, item_y, button_width,
        button_height, font_size, mouse_pos, mouse_clicked,
        shop->get_ball_speed_cost(), shop->ball_speed_level,
        [shop]() { shop->purchase_ball_speed(); },
        "Ball Speed (Lv " + std::to_string(shop->ball_speed_level) + ")");

    item_y += item_spacing;
    item_y = render_shop_button(
        shop, shop_x, shop_width, shop_padding, item_y, button_width,
        button_height, font_size, mouse_pos, mouse_clicked,
        shop->get_ball_damage_cost(), shop->ball_damage_level,
        [shop]() { shop->purchase_ball_damage(); },
        "Ball Damage (Lv " + std::to_string(shop->ball_damage_level) + ")");

    item_y += item_spacing;
    render_shop_button(
        shop, shop_x, shop_width, shop_padding, item_y, button_width,
        button_height, font_size, mouse_pos, mouse_clicked,
        shop->get_new_ball_cost(), shop->ball_count,
        [shop]() { shop->purchase_new_ball(); },
        "New Ball (x" + std::to_string(shop->ball_count) + ")");
  }

  float render_shop_button(IsShopManager *shop, float shop_x, float shop_width,
                           float shop_padding, float item_y, float button_width,
                           float button_height, float font_size,
                           raylib::Vector2 mouse_pos, bool mouse_clicked,
                           int cost, int level_or_count,
                           std::function<void()> purchase_func,
                           const std::string &label) const {
    bool can_afford = shop->pixels_collected >= cost;
    raylib::Rectangle button = {shop_x - shop_width / 2.0f + shop_padding,
                                item_y, button_width, button_height};
    bool hovered = raylib::CheckCollisionPointRec(mouse_pos, button);

    raylib::Color bg_color = get_button_bg_color(hovered, can_afford);
    raylib::DrawRectangleRec(button, bg_color);

    raylib::Color text_color = can_afford ? raylib::WHITE : raylib::GRAY;
    std::string button_text = label + " - " + std::to_string(cost) + " pixels";
    raylib::DrawTextEx(uiFont, button_text.c_str(),
                       {shop_x - shop_width / 2.0f + shop_padding + 5.0f,
                        item_y + (button_height - font_size) / 2.0f},
                       font_size, 1.0f, text_color);

    if (mouse_clicked && hovered && can_afford) {
      purchase_func();
    }

    return item_y;
  }

  raylib::Color get_button_bg_color(bool hovered, bool can_afford) const {
    if (hovered) {
      return can_afford ? raylib::Color{100, 100, 100, 255}
                        : raylib::Color{60, 60, 60, 255};
    }
    return can_afford ? raylib::Color{80, 80, 80, 255}
                      : raylib::Color{40, 40, 40, 255};
  }

  void render_shop_hint(float padding_x, float padding_y, float line_spacing,
                        float font_size) const {
    float hint_y = padding_y + line_spacing * 2.0f;
    raylib::DrawTextEx(uiFont, "Press TAB to open shop", {padding_x, hint_y},
                       font_size * 0.8f, 1.0f, raylib::GRAY);
  }
};
