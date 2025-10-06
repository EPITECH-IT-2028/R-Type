#include "BoundarySystem.hpp"
#include "PositionComponent.hpp"
#include "RenderManager.hpp"
#include "SpriteComponent.hpp"
#include "raylib.h"

void ecs::BoundarySystem::update(float deltaTime) {
  (void)deltaTime;
  for (auto const &entity : _entities) {
    auto &position = _ecsManager.getComponent<ecs::PositionComponent>(entity);
    auto const &sprite = _ecsManager.getComponent<ecs::SpriteComponent>(entity);

    float entityWidth = sprite.sourceRect.width + 16;
    float entityHeight = sprite.sourceRect.height + 16;

    const float worldMinX = 0.0f;
    const float worldMaxX = renderManager::WINDOW_WIDTH;
    const float worldMinY = 0.0f;
    const float worldMaxY = renderManager::WINDOW_HEIGHT;

    // Check horizontal boundaries
    if (position.x < worldMinX) {
      position.x = worldMinX;
    } else if (position.x + entityWidth > worldMaxX) {
      position.x = worldMaxX - entityWidth;
    }

    // Check vertical boundaries
    if (position.y < worldMinY) {
      position.y = worldMinY;
    } else if (position.y + entityHeight > worldMaxY) {
      position.y = worldMaxY - entityHeight;
    }
  }
}
