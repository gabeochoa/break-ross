#pragma once

#include "components.h"
#include <afterhours/ah.h>

afterhours::Entity &make_ball(vec2 position, vec2 velocity, float radius,
                              int damage);

afterhours::Entity &make_square(vec2 position, float size, float speed);

void setup_game();
