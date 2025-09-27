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
    if (!_ecsManager->hasComponent<ecs::RenderComponent>(entity)) {
      continue;
    }

    auto &renderComp = _ecsManager->getComponent<ecs::RenderComponent>(entity);
    
    std::string sprite = renderComp.sprite;
    std::cout << "Sprite path: " << sprite << std::endl;
    if (sprite.empty()) {
      continue;
    }

    float width = renderComp.width;
    float height = renderComp.height;
    float offsetX = renderComp.offsetX;
    float offsetY = renderComp.offsetY;

    float posX = _ecsManager->getComponent<ecs::PositionComponent>(entity).x;
    float posY = _ecsManager->getComponent<ecs::PositionComponent>(entity).y;

    Texture2D texture = LoadTexture(sprite.c_str());
    if (texture.id == 0) {
        std::cerr << "Failed to load texture: " << sprite << std::endl;
        continue;
    }
    Rectangle sourceRec = {0.0f, 0.0f, width, height};
    Rectangle destRec = {posX + offsetX, posY + offsetY, width > 0 ? width : static_cast<float>(texture.width), height > 0 ? height : static_cast<float>(texture.height)};
    Vector2 origin = {0.0f, 0.0f};

    DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, WHITE);
  }
}
