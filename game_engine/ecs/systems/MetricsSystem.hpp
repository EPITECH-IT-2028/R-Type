#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

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