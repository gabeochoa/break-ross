#pragma once

#include "log.h"
#include "rl.h"
#include "std_include.h"
#include <afterhours/ah.h>
#include <magic_enum/magic_enum.hpp>

struct Transform : afterhours::BaseComponent {
  vec2 position{0.f, 0.f};
  vec2 velocity{0.f, 0.f};
  vec2 size{0.f, 0.f};

  Transform() = default;
  Transform(vec2 pos, vec2 vel, vec2 sz)
      : position(pos), velocity(vel), size(sz) {}
};

enum struct ColliderTag : afterhours::TagId {
  Circle = 0,
  Rect = 1,
};

struct HasHealth : afterhours::BaseComponent {
  int max_amount{0};
  int amount{0};

  HasHealth() = default;
  HasHealth(int max_amount_in)
      : max_amount(max_amount_in), amount(max_amount_in) {}
  HasHealth(int max_amount_in, int amount_in)
      : max_amount(max_amount_in), amount(amount_in) {}
};

struct CanDamage : afterhours::BaseComponent {
  afterhours::EntityID id;
  int amount;

  CanDamage() = default;
  CanDamage(afterhours::EntityID id_in, int amount_in)
      : id(id_in), amount(amount_in) {}
};

struct IsShopManager : afterhours::BaseComponent {
  int ball_cost;
  int ball_damage;
  int pixels;

  IsShopManager() = default;
  IsShopManager(int cost, int damage, int pixels_in)
      : ball_cost(cost), ball_damage(damage), pixels(pixels_in) {}
};
