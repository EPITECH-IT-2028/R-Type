#include "InterpolationSystem.hpp"
#include <raylib.h>
#include "PositionComponent.hpp"
#include "StateHistoryComponent.hpp"

void ecs::InterpolationSystem::update(float deltaTime) {
  (void)deltaTime;  // We use absolute time from GetTime()

  double currentTime = GetTime();

  for (const auto &entity : _entities) {
    auto &stateHistory =
        _ecsManager.getComponent<StateHistoryComponent>(entity);
    auto &position = _ecsManager.getComponent<PositionComponent>(entity);

    EntityState state0, state1;
    float alpha;

    if (getInterpolatedStates(stateHistory, currentTime, state0, state1,
                              alpha)) {
      position.x = lerp(state0.x, state1.x, alpha);
      position.y = lerp(state0.y, state1.y, alpha);
    }
  }
}

/**
 * @brief Linear interpolation between two values.
 *
 * @param a Start value.
 * @param b End value.
 * @param t Interpolation factor (0.0 to 1.0).
 * @return Interpolated value.
 */
float ecs::InterpolationSystem::lerp(float a, float b, float t) const {
  return a + (b - a) * t;
}

/**
 * @brief Get two states to interpolate between for a given render time.
 *
 * Returns the two states that bracket the target render time, which is
 * current time minus interpolation delay.
 *
 * @param stateHistory The state history component.
 * @param currentTime Current time in seconds.
 * @param state0 [out] Earlier state (or current if only one exists).
 * @param state1 [out] Later state (or current if only one exists).
 * @param alpha [out] Interpolation factor between state0 and state1
 * (0.0-1.0).
 * @return true if valid states were found, false otherwise.
 */
bool ecs::InterpolationSystem::getInterpolatedStates(const StateHistoryComponent &stateHistory,
                            double currentTime, EntityState &state0,
                            EntityState &state1, float &alpha) const {
  const auto &states = stateHistory.states;

  if (states.empty()) {
    return false;
  }

  double renderTime = currentTime - INTERPOLATION_DELAY;

  if (states.size() == 1) {
    state0 = states[0];
    state1 = states[0];
    alpha = 0.0f;
    return true;
  }

  for (size_t i = 0; i < states.size() - 1; ++i) {
    if (states[i].timestamp <= renderTime &&
        renderTime <= states[i + 1].timestamp) {
      state0 = states[i];
      state1 = states[i + 1];

      double timeDiff = state1.timestamp - state0.timestamp;
      if (timeDiff > 0.0) {
        alpha = static_cast<float>((renderTime - state0.timestamp) / timeDiff);
      } else {
        alpha = 0.0f;
      }
      return true;
    }
  }

  if (renderTime < states.front().timestamp) {
    state0 = states.front();
    state1 = states.front();
    alpha = 0.0f;
    return true;
  }

  state0 = states[states.size() - 2];
  state1 = states.back();
  alpha = 1.0f;
  return true;
}
