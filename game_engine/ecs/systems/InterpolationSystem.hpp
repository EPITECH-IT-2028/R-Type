#pragma once

#include <raylib.h>
#include "ECSManager.hpp"
#include "PositionComponent.hpp"
#include "StateHistoryComponent.hpp"
#include "System.hpp"

namespace ecs {
  /**
   * @brief System that interpolates entity positions based on buffered state
   * updates.
   *
   * For each entity with a StateHistoryComponent, calculates a smoothly
   * interpolated position between recent server updates, rendering entities
   * slightly in the past to ensure fluid movement.
   */
  class InterpolationSystem : public System {
   public:
    explicit InterpolationSystem(
        ECSManager &ecsManager = ECSManager::getInstance())
        : _ecsManager(ecsManager) {
    }

    void update(float deltaTime) override;

   private:
    ECSManager &_ecsManager;
    float linterpolation(float a, float b, float t) const;
    bool getInterpolatedStates(const StateHistoryComponent &stateHistory,
                               double currentTime, EntityState &state0,
                               EntityState &state1, float &alpha) const;
  };
}  // namespace ecs