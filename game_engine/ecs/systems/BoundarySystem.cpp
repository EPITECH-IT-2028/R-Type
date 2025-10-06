#include "BoundarySystem.hpp"
#include "PositionComponent.hpp"
#include "RenderManager.hpp"
#include "SpriteComponent.hpp"
#include "raylib.h"

void ecs::BoundarySystem::update(float deltaTime) {
  (void)deltaTime;
  const float worldMinX = 0.0f;
  const float worldMaxX = renderManager::WINDOW_WIDTH;
  const float worldMinY = 0.0f;
  const float worldMaxY = renderManager::WINDOW_HEIGHT;

  for (auto const &entity : _entities) {
    auto &position = _ecsManager.getComponent<ecs::PositionComponent>(entity);
    auto const &sprite = _ecsManager.getComponent<ecs::SpriteComponent>(entity);

    float entityWidth = sprite.sourceRect.width + ENTITY_MARGIN_X;
    float entityHeight = sprite.sourceRect.height + ENTITY_MARGIN_Y;

    if (position.x < worldMinX) {
      position.x = worldMinX;
    } else if (position.x + entityWidth > worldMaxX) {
      position.x = worldMaxX - entityWidth;
    }

    if (position.y < worldMinY) {
      position.y = worldMinY;
    } else if (position.y + entityHeight > worldMaxY) {
      position.y = worldMaxY - entityHeight;
    }
  }
}
