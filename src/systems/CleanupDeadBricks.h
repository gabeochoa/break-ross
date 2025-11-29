#pragma once

#include "../components.h"
#include "../eq.h"
#include <afterhours/ah.h>

struct CleanupDeadBricks : afterhours::System<HasHealth> {
  virtual void for_each_with(afterhours::Entity &entity, HasHealth &hasHealth,
                             float) override {
    if (hasHealth.amount <= 0) {
      entity.cleanup = true;
    }
  }
};
