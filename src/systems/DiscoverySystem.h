#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../log.h"
#include "MapRevealSystem.h"
#include <afterhours/ah.h>
#include <cmath>

struct DiscoverySystem
    : afterhours::System<
          Transform,
          afterhours::tags::Any<ColliderTag::Square, ColliderTag::Circle>> {
  virtual void for_each_with(afterhours::Entity &, Transform &transform,
                             float) override {
    FogOfWar *fog = afterhours::EntityHelper::get_singleton_cmp<FogOfWar>();
    if (!fog) {
      return;
    }

    float reveal_radius = fog->reveal_radius;

    for (PointOfInterest &poi : afterhours::EntityQuery()
                                    .whereHasComponent<PointOfInterest>()
                                    .gen_as<PointOfInterest>()) {
      if (poi.is_discovered) {
        continue;
      }

      float dx = transform.position.x - poi.position.x;
      float dy = transform.position.y - poi.position.y;
      float dist_sq = dx * dx + dy * dy;
      float reveal_radius_sq = reveal_radius * reveal_radius;

      if (dist_sq <= reveal_radius_sq) {
        poi.is_discovered = true;

        IsShopManager *shop =
            afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
        if (shop) {
          shop->pixels_collected += poi.reward_amount;
          log_info("Discovered POI! Type: %d, Reward: %d pixels",
                   static_cast<int>(poi.poi_type), poi.reward_amount);
        }
      }
    }
  }
};

