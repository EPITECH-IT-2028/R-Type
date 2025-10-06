#include "BoundarySystem.hpp"
#include <cmath>
#include "PositionComponent.hpp"
#include "RenderManager.hpp"
#include "ScaleComponent.hpp"
#include "SpriteComponent.hpp"
#include "raylib.h"

void ecs::BoundarySystem::update(float deltaTime) {
  (void)deltaTime;
  const float worldMinX = 0.0f;
  const float worldMaxX = renderManager::WINDOW_WIDTH;
  const float worldMinY = 0.0f;
  const float worldMaxY = renderManager::WINDOW_HEIGHT;
  const float ENTITY_MARGIN_X = 2.0f;
  const float ENTITY_MARGIN_Y = 2.0f;

  for (auto const &entity : _entities) {
    auto &position = _ecsManager.getComponent<ecs::PositionComponent>(entity);
    auto const &sprite = _ecsManager.getComponent<ecs::SpriteComponent>(entity);

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    if (_ecsManager.hasComponent<ecs::ScaleComponent>(entity)) {
      auto const &scale = _ecsManager.getComponent<ecs::ScaleComponent>(entity);
      scaleX = scale.scaleX;
      scaleY = scale.scaleY;
    }

    float entityWidth =
        std::abs(sprite.sourceRect.width) * scaleX + ENTITY_MARGIN_X;
    float entityHeight =
        std::abs(sprite.sourceRect.height) * scaleY + ENTITY_MARGIN_Y;

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
