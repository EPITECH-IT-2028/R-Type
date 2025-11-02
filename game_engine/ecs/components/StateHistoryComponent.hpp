#pragma once

#include <cstdint>
#include <deque>

namespace ecs {
  constexpr size_t MAX_INTERPOLATION_STATES = 10;
  constexpr double INTERPOLATION_DELAY = 0.05;
  constexpr float MAX_EXTRAPOLATION = 1.15f;

  /**
   * @brief Represents a single state event of an entity at a specific time.
   */
  struct EntityState {
    float x;
    float y;
    double timestamp;
  };

  /**
   * @brief Component that stores a history of entity states for interpolation.
   *
   * Maintains a buffer of recent state updates from the server, allowing
   * smooth interpolation between states to eliminate jerky movement.
   */
  struct StateHistoryComponent {
    std::deque<EntityState> states;
    mutable std::shared_ptr<std::mutex> mutex = std::make_shared<std::mutex>();
  };
}  // namespace ecs
