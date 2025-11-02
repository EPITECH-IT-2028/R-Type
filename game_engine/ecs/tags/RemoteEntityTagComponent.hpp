#pragma once

namespace ecs {
  /**
   * @brief Tag component marking an entity as remotely controlled.
   *
   * Entities with this tag represent remote players or NPCs whose state
   * is controlled by the server. These entities should use interpolation
   * for smooth movement rendering.
   */
  struct RemoteEntityTagComponent {};
}  // namespace ecs