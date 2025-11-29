#pragma once

#include "../components.h"
#include "../eq.h"
#include "../game_constants.h"
#include "../log.h"
#include <afterhours/ah.h>

struct CleanupDeadBricks : afterhours::System<Transform, HasHealth> {
  IsShopManager *shop = nullptr;
  IsPhotoReveal *photo_reveal = nullptr;

  virtual void once(float) override {
    afterhours::Entity &shop_entity =
        afterhours::EntityHelper::get_singleton<IsShopManager>();
    if (!shop_entity.has<IsShopManager>()) {
      log_error("IsShopManager singleton not found");
    }
    shop = &shop_entity.get<IsShopManager>();

    afterhours::Entity &photo_entity =
        afterhours::EntityHelper::get_singleton<IsPhotoReveal>();
    if (!photo_entity.has<IsPhotoReveal>()) {
      log_error("IsPhotoReveal singleton not found");
    }
    photo_reveal = &photo_entity.get<IsPhotoReveal>();
  }

  virtual void for_each_with(afterhours::Entity &entity, Transform &transform,
                             HasHealth &hasHealth, float) override {
    if (hasHealth.amount > 0) {
      return;
    }

    shop->pixels_collected += 1;

    int grid_x = static_cast<int>(
        (transform.position.x - game_constants::BRICK_START_X) /
        game_constants::BRICK_CELL_SIZE);
    int grid_y = static_cast<int>(
        (transform.position.y - game_constants::BRICK_START_Y) /
        game_constants::BRICK_CELL_SIZE);

    photo_reveal->set_revealed(grid_x, grid_y);
    photo_reveal->rebuild_merged_rects();

    entity.cleanup = true;
  }
};
