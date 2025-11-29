#pragma once

#include "../components.h"
#include "../eq.h"
#include <afterhours/ah.h>

struct UpdateCarUpgrades : afterhours::System<IsShopManager> {
  int last_speed_level{-1};
  int last_damage_level{-1};

  virtual void once(float) override {
    IsShopManager *shop =
        afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
    last_speed_level = shop->car_speed_level;
    last_damage_level = shop->car_damage_level;
  }

  virtual void for_each_with(afterhours::Entity &, IsShopManager &shop,
                             float) override {
    bool speed_changed = shop.car_speed_level != last_speed_level;
    bool damage_changed = shop.car_damage_level != last_damage_level;

    if (!speed_changed && !damage_changed) {
      return;
    }

    int old_speed_level = last_speed_level;

    last_speed_level = shop.car_speed_level;
    last_damage_level = shop.car_damage_level;

    if (speed_changed) {
      float speed_multiplier = shop.get_car_speed_multiplier();

      float old_multiplier =
          old_speed_level > 0 ? 1.0f + ((old_speed_level - 1) * 0.2f) : 1.0f;
      float speed_scale = speed_multiplier / old_multiplier;

      for (Transform &transform : afterhours::EntityQuery()
                                      .whereHasTag(ColliderTag::Circle)
                                      .whereHasComponent<Transform>()
                                      .gen_as<Transform>()) {
        transform.velocity.x *= speed_scale;
        transform.velocity.y *= speed_scale;
      }
    }

    if (damage_changed) {
      int damage_value = shop.get_car_damage_value();

      for (CanDamage &can_damage : afterhours::EntityQuery()
                                       .whereHasTag(ColliderTag::Circle)
                                       .whereHasComponent<CanDamage>()
                                       .gen_as<CanDamage>()) {
        can_damage.amount = damage_value;
      }
    }
  }
};

