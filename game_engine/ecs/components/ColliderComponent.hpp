#pragma once

namespace ecs {

  /** @brief A simple 2D vector structure. */
  struct Vec2 {
      float x;
      float y;
  };
  /** @brief Component that defines a collider for an entity.
   * It can be an axis-aligned bounding box (AABB).
   */
  struct ColliderComponent {
      Vec2 center = {0.0f, 0.0f};
      Vec2 halfSize = {0.5f, 0.5f};
  };
}  // namespace ecs
