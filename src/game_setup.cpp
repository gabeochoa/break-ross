#include "game_setup.h"

#include "components.h"
#include "eq.h"
#include "settings.h"
#include <afterhours/ah.h>

static afterhours::Entity &make_ball(vec2 position, vec2 velocity, float radius,
                                     int damage) {
  afterhours::Entity &ball = afterhours::EntityHelper::createEntity();
  ball.addComponent<Transform>(position, velocity,
                               vec2{radius * 2.0f, radius * 2.0f});
  ball.enableTag(ColliderTag::Circle);
  ball.addComponent<CanDamage>(ball.id, damage);
  return ball;
}

static afterhours::Entity &make_brick(vec2 position, vec2 size, int health) {
  afterhours::Entity &brick = afterhours::EntityHelper::createEntity();
  brick.addComponent<Transform>(position, vec2{0, 0}, size);
  brick.enableTag(ColliderTag::Rect);
  brick.addComponent<HasHealth>(health, health);
  return brick;
}

void make_sophie() {
  afterhours::Entity &sophie =
      afterhours::EntityHelper::createPermanentEntity();
  sophie.addComponent<IsShopManager>(100, 1, 100);
  afterhours::EntityHelper::registerSingleton<IsShopManager>(sophie);
}

void setup_game() {
  make_sophie();

  IsShopManager *shop =
      afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();

  float screen_width = static_cast<float>(Settings::get().get_screen_width());
  float screen_height = static_cast<float>(Settings::get().get_screen_height());

  float radius = 10.0f;
  make_ball(vec2{screen_width / 2.0f, screen_height / 2.0f},
            vec2{200.0f, 200.0f}, radius, shop->ball_damage);

  constexpr int cols = 100;
  constexpr int rows = 50;
  constexpr float brick_size = 30.0f;
  constexpr float spacing = 5.0f;
  constexpr float start_x = 50.0f;
  constexpr float start_y = 50.0f;

  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      float x = start_x + col * (brick_size + spacing);
      float y = start_y + row * (brick_size + spacing);

      make_brick(vec2{x, y}, vec2{brick_size, brick_size}, 1);
    }
  }
}
