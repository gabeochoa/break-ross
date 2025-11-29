#pragma once

#include "components.h"
#include <afterhours/ah.h>
#include <algorithm>
#include <cmath>

struct EQ : afterhours::EntityQuery<EQ> {
  struct WhereNearby : afterhours::EntityQuery<EQ>::Modification {
    vec2 position;
    float radius;

    explicit WhereNearby(vec2 pos, float r) : position(pos), radius(r) {}

    bool operator()(const afterhours::Entity &entity) const override {
      if (!entity.has<Transform>()) {
        return false;
      }
      const Transform &transform = entity.get<Transform>();

      float left = transform.position.x;
      float right = transform.position.x + transform.size.x;
      float top = transform.position.y;
      float bottom = transform.position.y + transform.size.y;

      float expanded_left = position.x - radius;
      float expanded_right = position.x + radius;
      float expanded_top = position.y - radius;
      float expanded_bottom = position.y + radius;

      return !(right < expanded_left || left > expanded_right ||
               bottom < expanded_top || top > expanded_bottom);
    }
  };

  EQ &whereNearby(vec2 position, float radius) {
    return add_mod(new WhereNearby(position, radius));
  }
};
