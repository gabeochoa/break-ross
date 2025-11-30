#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../log.h"
#include "MapRevealSystem.h"
#include <afterhours/ah.h>

struct AutoRevealUnreachableFog : afterhours::System<FogOfWar> {
  bool has_auto_revealed{false};

  virtual void for_each_with(afterhours::Entity &, FogOfWar &fog,
                             float) override {
    RoadNetwork *road_network =
        afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
    invariant(road_network, "RoadNetwork singleton not found");

    if (!road_network->is_loaded) {
      return;
    }

    if (!fog.reachable_computed) {
      MapRevealSystem::compute_reachable_cells();
    }

    if (has_auto_revealed) {
      return;
    }

    if (fog.are_all_reachable_revealed()) {
      IsPhotoReveal *photo_reveal =
          afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
      invariant(photo_reveal, "IsPhotoReveal singleton not found");

      int unreachable_count = 0;
      for (int i = 0; i < game_constants::GRID_SIZE; ++i) {
        if (!fog.reachable_cells[i] && !fog.revealed_cells[i]) {
          int grid_x = i % game_constants::GRID_WIDTH;
          int grid_y = i / game_constants::GRID_WIDTH;
          fog.set_revealed(grid_x, grid_y);
          photo_reveal->set_revealed(grid_x, grid_y);
          unreachable_count++;
        }
      }

      if (unreachable_count > 0) {
        log_info(
            "AutoRevealUnreachableFog: Auto-revealed {} unreachable fog cells",
            unreachable_count);
      }
      has_auto_revealed = true;
    }
  }
};
