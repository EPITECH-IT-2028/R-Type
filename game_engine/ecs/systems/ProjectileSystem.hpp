#pragma once

#include "System.hpp"

namespace ecs {
  class ProjectileSystem : public System {
      ProjectileSystem() = default;
      ~ProjectileSystem() = default;

      void update(float dt) override;
  };
}  // namespace ecs
