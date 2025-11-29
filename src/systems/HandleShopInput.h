#pragma once

#include "../components.h"
#include "../eq.h"
#include "../rl.h"
#include <afterhours/ah.h>

struct HandleShopInput : afterhours::System<IsShopManager> {
  virtual void for_each_with(afterhours::Entity &, IsShopManager &shop,
                             float) override {
    if (raylib::IsKeyPressed(raylib::KEY_TAB)) {
      shop.shop_open = !shop.shop_open;
    }
  }
};
