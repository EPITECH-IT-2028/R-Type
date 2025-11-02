#include "InterpolationSystem.hpp"
#include <raylib.h>
#include <algorithm>
#include "PositionComponent.hpp"
#include "StateHistoryComponent.hpp"

void ecs::InterpolationSystem::update(float deltaTime) {
  (void)deltaTime;
  double currentTime = GetTime();

  for (const auto &entity : _entities) {
    auto &stateHistory =
        _ecsManager.getComponent<StateHistoryComponent>(entity);

    std::lock_guard<std::mutex> lock(*stateHistory.mutex);

    auto &position = _ecsManager.getComponent<PositionComponent>(entity);
    EntityState state0, state1;
    float alpha;

    if (getInterpolatedStatesInternal(stateHistory, currentTime, state0, state1,
                                      alpha)) {
      float dx = state1.x - state0.x;
      float dy = state1.y - state0.y;
      float distanceSquared = dx * dx + dy * dy;
      float maxAlpha = MAX_EXTRAPOLATION;

      if (distanceSquared > 400.0f) {
        maxAlpha = 0.95f;
      } else if (distanceSquared > 100.0f) {
        maxAlpha = 1.0f;
      } else if (distanceSquared > 25.0f) {
        maxAlpha = 1.05f;
      }

      float clampedAlpha = std::min(alpha, maxAlpha);
      position.x = linterpolation(state0.x, state1.x, clampedAlpha);
      position.y = linterpolation(state0.y, state1.y, clampedAlpha);
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
float ecs::InterpolationSystem::linterpolation(float a, float b,
                                               float t) const {
  return a + (b - a) * t;
}

/**
 * @brief Get two states to interpolate between for a given render time.
 * This version acquires the mutex lock for thread-safe access.
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
bool ecs::InterpolationSystem::getInterpolatedStates(
    const StateHistoryComponent &stateHistory, double currentTime,
    EntityState &state0, EntityState &state1, float &alpha) const {
  std::lock_guard<std::mutex> lock(*stateHistory.mutex);
  return getInterpolatedStatesInternal(stateHistory, currentTime, state0,
                                       state1, alpha);
}

/**
 * @brief Internal version of getInterpolatedStates that assumes the mutex is
 * already locked.
 *
 * This method MUST only be called when the caller already holds the
 * StateHistoryComponent's mutex lock. It performs the same interpolation logic
 * as getInterpolatedStates but without acquiring the lock, preventing deadlock.
 *
 * @param stateHistory The state history component (mutex MUST already be locked
 * by caller).
 * @param currentTime Current time in seconds.
 * @param state0 [out] Earlier state (or current if only one exists).
 * @param state1 [out] Later state (or current if only one exists).
 * @param alpha [out] Interpolation factor between state0 and state1
 * (0.0-1.0).
 * @return true if valid states were found, false otherwise.
 */
bool ecs::InterpolationSystem::getInterpolatedStatesInternal(
    const StateHistoryComponent &stateHistory, double currentTime,
    EntityState &state0, EntityState &state1, float &alpha) const {
  const auto &states = stateHistory.states;
  if (states.empty()) {
    return false;
  }
  if (states.size() == 1) {
    state0 = states[0];
    state1 = states[0];
    alpha = 0.0f;
    return true;
  }

  state0 = states.front();
  state1 = states.back();
  double timeDiff = state1.timestamp - state0.timestamp;

  if (timeDiff < 0.001) {
    state0 = state1;
    alpha = 0.0f;
    return true;
  }

  double timeSinceFirst = currentTime - INTERPOLATION_DELAY - state0.timestamp;
  alpha = static_cast<float>(timeSinceFirst / timeDiff);
  alpha = std::max(0.0f, std::min(MAX_EXTRAPOLATION, alpha));
  return true;
}
