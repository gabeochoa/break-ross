#pragma once

#include "../components.h"
#include "../eq.h"
#include "../log.h"
#include <afterhours/ah.h>

// Drives the region-completion loop: award a one-time bonus when the current
// region is fully imaged, and reset the map's reveal state when the player
// expands into the next region so the fleet has fresh streets to map.
struct TerritoryProgress : afterhours::System<IsShopManager> {
  int last_expand_level{0};
  int completion_awarded_level{-1};

  virtual void for_each_with(afterhours::Entity &, IsShopManager &shop,
                             float) override {
    IsPhotoReveal *photo_reveal =
        afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
    invariant(photo_reveal, "IsPhotoReveal singleton not found");

    // Expanded into a new region: clear the reveal so cars re-map it.
    if (shop.expand_level > last_expand_level) {
      last_expand_level = shop.expand_level;
      reset_region(photo_reveal);
    }

    // Region fully imaged: award the completion bonus once per region.
    if (!shop.region_complete && completion_awarded_level < shop.expand_level &&
        photo_reveal->get_reveal_percentage() >= 99.0f) {
      shop.region_complete = true;
      completion_awarded_level = shop.expand_level;
      shop.pixels_collected += IsShopManager::REGION_COMPLETE_BONUS;
      log_info("TerritoryProgress: {} region mapped — +{} bonus",
               shop.get_scope_name(), IsShopManager::REGION_COMPLETE_BONUS);
    }
  }

private:
  static void reset_region(IsPhotoReveal *photo_reveal) {
    photo_reveal->revealed_cells.reset();
    photo_reveal->merged_rects.clear();
    photo_reveal->merged_rects_dirty = true;
    photo_reveal->mask_texture_dirty = true;

    RoadNetwork *road_network =
        afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
    invariant(road_network, "RoadNetwork singleton not found");
    std::fill(road_network->visited_segments.begin(),
              road_network->visited_segments.end(), false);
  }
};
