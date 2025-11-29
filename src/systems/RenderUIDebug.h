#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game.h"
#include "../input_mapping.h"
#include "../log.h"
#include "../rl.h"
#include "../settings.h"
#include <afterhours/ah.h>
#include <afterhours/src/plugins/ui.h>
#include <fmt/format.h>
#include <string>

using namespace afterhours::ui;
using namespace afterhours::ui::imm;

struct RenderUIDebug
    : afterhours::System<afterhours::ui::UIContext<InputAction>> {
  virtual void for_each_with(afterhours::Entity &entity,
                             afterhours::ui::UIContext<InputAction> &context,
                             float) override {
    // Always show debug info
    float fontSize = 16.0f;
    float y = 10.0f;
    float x = 10.0f;
    float lineHeight = fontSize + 5.0f;

    // Show mouse position
    std::string mouse_text = fmt::format("Mouse: ({}, {})", context.mouse_pos.x,
                                         context.mouse_pos.y);
    raylib::DrawText(mouse_text.c_str(), (int)x, (int)y, (int)fontSize,
                     raylib::WHITE);
    y += lineHeight;

    // Show render commands count
    std::string render_cmds_text =
        fmt::format("Render commands: {}", context.render_cmds.size());
    raylib::DrawText(render_cmds_text.c_str(), (int)x, (int)y, (int)fontSize,
                     raylib::WHITE);
    y += lineHeight;

    // Show root entity info
    std::string root_text = fmt::format("Root entity: {}", context.ROOT);
    raylib::DrawText(root_text.c_str(), (int)x, (int)y, (int)fontSize,
                     raylib::WHITE);
    y += lineHeight;

    // Show hot/active/focus IDs
    std::string focus_text =
        fmt::format("Focus: {}, Hot: {}, Active: {}", context.focus_id,
                    context.hot_id, context.active_id);
    raylib::DrawText(focus_text.c_str(), (int)x, (int)y, (int)fontSize,
                     raylib::WHITE);
    y += lineHeight;

    // Show first few render commands
    int max_show = 10;
    for (size_t i = 0; i < context.render_cmds.size() && i < max_show; ++i) {
      const auto &cmd = context.render_cmds[i];
      std::string cmd_text =
          fmt::format("  Cmd[{}]: id={}, layer={}", i, cmd.id, cmd.layer);
      raylib::DrawText(cmd_text.c_str(), (int)x, (int)y, (int)fontSize,
                       raylib::WHITE);
      y += lineHeight;

      // Try to get entity and show its info
      try {
        afterhours::Entity &ent =
            afterhours::EntityHelper::getEntityForIDEnforce(cmd.id);
        if (ent.has<UIComponent>()) {
          const UIComponent &cmp = ent.get<UIComponent>();
          // Get final position including translate
          RectangleType final_rect = cmp.rect();
          if (ent.has<HasUIModifiers>()) {
            final_rect = ent.get<HasUIModifiers>().apply_modifier(final_rect);
          }
          std::string cmp_text = fmt::format(
              "    -> x={:.1f}, y={:.1f}, w={:.1f}, h={:.1f}", final_rect.x,
              final_rect.y, final_rect.width, final_rect.height);
          raylib::DrawText(cmp_text.c_str(), (int)(x + 20), (int)y,
                           (int)(fontSize - 2), raylib::YELLOW);
          y += lineHeight - 2;

          if (ent.has<HasLabel>()) {
            const HasLabel &label = ent.get<HasLabel>();
            std::string label_text =
                fmt::format("    -> Label: '{}'", label.label);
            raylib::DrawText(label_text.c_str(), (int)(x + 20), (int)y,
                             (int)(fontSize - 2), raylib::GREEN);
            y += lineHeight - 2;
          }

          if (ent.has<afterhours::HasColor>()) {
            const afterhours::HasColor &color = ent.get<afterhours::HasColor>();
            afterhours::Color col = color.color();
            std::string color_text = fmt::format(
                "    -> Color: ({}, {}, {}, {})", col.r, col.g, col.b, col.a);
            raylib::DrawText(color_text.c_str(), (int)(x + 20), (int)y,
                             (int)(fontSize - 2), raylib::SKYBLUE);
            y += lineHeight - 2;
          }
        }
      } catch (...) {
        // Entity not found, skip
      }
    }

    if (context.render_cmds.size() > (size_t)max_show) {
      std::string more_text = fmt::format(
          "  ... and {} more", context.render_cmds.size() - max_show);
      raylib::DrawText(more_text.c_str(), (int)x, (int)y, (int)fontSize,
                       raylib::WHITE);
    }
  }
};
