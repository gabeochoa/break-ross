#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../log.h"
#include "MapRevealSystem.h"
#include <afterhours/ah.h>
#include <cmath>

struct RevealFogOfWar
    : afterhours::System<Transform,
                         afterhours::tags::Any<ColliderTag::Square, ColliderTag::Circle>> {
  virtual void for_each_with(afterhours::Entity &, Transform &transform,
                             float) override {
    FogOfWar *fog = afterhours::EntityHelper::get_singleton_cmp<FogOfWar>();
    if (!fog) {
      return;
    }

    MapRevealSystem::reveal_position(transform.position, fog->reveal_radius);
  }
};
