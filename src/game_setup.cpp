#include "game_setup.h"

#include "components.h"
#include "eq.h"
#include "game_constants.h"
#include "render_backend.h"
#include "settings.h"
#include <afterhours/ah.h>
#include <afterhours/src/plugins/autolayout.h>
#include <afterhours/src/plugins/files.h>

static afterhours::Entity &make_ball(vec2 position, vec2 velocity, float radius,
                                     int damage) {
  afterhours::Entity &ball = afterhours::EntityHelper::createEntity();
  ball.addComponent<Transform>(position, velocity,
                               vec2{radius * 2.0f, radius * 2.0f});
  ball.enableTag(ColliderTag::Circle);
  ball.addComponent<CanDamage>(ball.id, damage);
  return ball;
}

template <typename Component, typename... Args>
static void addIfMissing(afterhours::Entity &entity, Args &&...args) {
  if (!entity.has<Component>()) {
    entity.addComponent<Component>(std::forward<Args>(args)...);
    afterhours::EntityHelper::registerSingleton<Component>(entity);
  }
}

static afterhours::Entity &get_sophie() {
  afterhours::Entity &sophie =
      afterhours::EntityHelper::get_singleton<afterhours::ui::AutoLayoutRoot>();
  if (!sophie.has<afterhours::ui::AutoLayoutRoot>()) {
    log_error("Sophie entity not found - make_singleton() must be called "
              "before setup_game()");
  }
  return sophie;
}

void setup_game() {
  afterhours::Entity &sophie = get_sophie();

  addIfMissing<IsShopManager>(sophie, 100, 1, 100);
  addIfMissing<IsPhotoReveal>(sophie, game_constants::BRICK_CELL_SIZE);
  addIfMissing<BrickGrid>(sophie);

  IsPhotoReveal *photo_reveal =
      afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
  if (photo_reveal && !photo_reveal->is_loaded) {
    std::filesystem::path photo_path = afterhours::files::get_resource_path(
        "images/photos", "test_photo_500x500.png");
    photo_reveal->photo_texture =
        render_backend::LoadTexture(photo_path.string().c_str());
    photo_reveal->is_loaded = true;
  }

  IsShopManager *shop =
      afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();

  float screen_width = static_cast<float>(Settings::get().get_screen_width());
  float screen_height = static_cast<float>(Settings::get().get_screen_height());

  float radius = 10.0f;
  make_ball(vec2{screen_width / 2.0f, screen_height / 2.0f},
            vec2{200.0f, 200.0f}, radius, shop->ball_damage);

  for (int i = 0; i < 100; ++i) {
    make_ball(vec2{screen_width / 2.0f + i * 10.0f, screen_height / 2.0f},
              vec2{200.0f, 200.0f}, radius, shop->ball_damage);
  }

  BrickGrid *brick_grid =
      afterhours::EntityHelper::get_singleton_cmp<BrickGrid>();
  if (brick_grid) {
    for (int row = 0; row < game_constants::GRID_HEIGHT; ++row) {
      for (int col = 0; col < game_constants::GRID_WIDTH; ++col) {
        brick_grid->set_health(col, row, 1);
      }
    }
  }
}
