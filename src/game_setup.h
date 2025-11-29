#pragma once

#include "components.h"
#include <afterhours/ah.h>

afterhours::Entity &make_ball(vec2 position, vec2 velocity, float radius,
                              int damage);

void setup_game();
