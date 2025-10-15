#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  class ProjectileSystem : public System {
    public:
      explicit ProjectileSystem(
          ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {
      }

      /**
       * @brief Sets the optional ECSManager pointer used by this system.
       *
       * Assigns the internal pointer that this system will use for ECS operations; passing
       * `nullptr` clears the pointer.
       *
       * @param ecsManager Pointer to an ECSManager instance, or `nullptr` to unset.
       */
      void setECSManager(ECSManager *ecsManager) {
        _ecsManagerPtr = ecsManager;
      }

      void update(float dt) override;
      void moveBasics(const Entity &entity, float dt);

    private:
      ECSManager &_ecsManager;
      ECSManager *_ecsManagerPtr = nullptr;
  };
}  // namespace ecs