#include "RenderSystem.hpp"
#include "RenderComponent.hpp"
#include "PositionComponent.hpp"
#include <iostream>
#include "raylib.h"

void ecs::RenderSystem::update(__attribute__((unused)) float deltaTime) {
  render();
}

void ecs::RenderSystem::render() {
  for (Entity entity : _entities) {

    auto &renderComp = _ecsManager->getComponent<ecs::RenderComponent>(entity);
    
    if (renderComp._texturePath.empty()) {
      continue;
    }

    float width = renderComp._width;
    float height = renderComp._height;
    float offsetX = renderComp._offsetX;
    float offsetY = renderComp._offsetY;

    float posX = _ecsManager->getComponent<ecs::PositionComponent>(entity).x;
    float posY = _ecsManager->getComponent<ecs::PositionComponent>(entity).y;

    Rectangle sourceRec = {0.0f, 0.0f, width, height};
    Rectangle destRec = {posX + offsetX, posY + offsetY, width, height};
    Vector2 origin = {0.0f, 0.0f};

    DrawTexturePro(renderComp._texture, sourceRec, destRec, origin, 0.0f, WHITE);
  }
}
