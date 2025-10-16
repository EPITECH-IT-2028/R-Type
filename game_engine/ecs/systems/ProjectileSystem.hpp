#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  class ProjectileSystem : public System {
    public:
      ProjectileSystem() : _ecsManagerPtr(nullptr) {
      }

      ~ProjectileSystem() override = default;

      /**
       * @brief Sets the ECSManager instance that this system will use.
       *
       * @param ecsManager Pointer to the ECSManager to associate with this
       * system.
       */
      void setECSManager(ECSManager *ecsManager) {
        _ecsManagerPtr = ecsManager;
      }

      ECSManager *getECSManager() const {
        return _ecsManagerPtr;
      }

      void update(float dt) override;
      void moveBasics(const Entity &entity, float dt);

    private:
      ECSManager *_ecsManagerPtr;
  };
}  // namespace ecs
