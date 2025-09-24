#include "RenderSystem.hpp"
#include "RenderComponent.hpp"
#include "raylib.h"

void ecs::RenderSystem::update(__attribute__((unused)) float deltaTime) {
  render();
}

void ecs::RenderSystem::render() {
  for (Entity entity : _entities) {
    if (!_ecsManager->hasComponent<ecs::RenderComponent>(entity))
      continue;

      auto &renderComp = _ecsManager->getComponent<ecs::RenderComponent>(entity);

      std::string sprite = renderComp.sprite;

      if (sprite.empty()) {
        continue;
      }

      float posX = _ecsManager->getComponent<ecs::PositionComponent>(entity).x;
      float posY = _ecsManager->getComponent<ecs::PositionComponent>(entity).y;

      Texture2D texture = LoadTexture(sprite.c_str());
      DrawTexture(texture, posX, posY, WHITE);
      UnloadTexture(texture);
  }
}
