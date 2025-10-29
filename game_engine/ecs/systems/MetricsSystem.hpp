#pragma once

#include <string>
#include <unordered_map>
#include "ECSManager.hpp"
#include "System.hpp"
#include "raylib.h"

namespace ecs {
  class MetricsSystem : public System {
    public:
      explicit MetricsSystem(
          ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {
      }
      ~MetricsSystem() noexcept;

      void update(float deltaTime) override;

    private:
      ECSManager &_ecsManager;
  };
}  // namespace ecs