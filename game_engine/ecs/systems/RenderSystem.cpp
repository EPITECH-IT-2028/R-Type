#include "RenderSystem.hpp"
#include <iostream>
#include "PositionComponent.hpp"
#include "RenderComponent.hpp"
#include "raylib.h"

ecs::RenderSystem::~RenderSystem() {
  for (auto &pair : _textureCache) {
    UnloadTexture(pair.second);
  }
}

void ecs::RenderSystem::update(__attribute__((unused)) float deltaTime) {
  render();
}

void ecs::RenderSystem::render() {
  for (Entity entity : _entities) {
    auto &renderComp = _ecsManager->getComponent<ecs::RenderComponent>(entity);
    const std::string &path = renderComp._texturePath;

    if (path.empty())
      continue;

    if (_textureCache.find(path) == _textureCache.end()) {
      Texture2D newTexture = LoadTexture(path.c_str());

      if (newTexture.id == 0) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        continue;
      }

      _textureCache[path] = newTexture;
    }

    Texture2D &texture = _textureCache[path];

    float width = renderComp._width;
    float height = renderComp._height;
    float offsetX = renderComp._offsetX;
    float offsetY = renderComp._offsetY;

    float posX = _ecsManager->getComponent<ecs::PositionComponent>(entity).x;
    float posY = _ecsManager->getComponent<ecs::PositionComponent>(entity).y;

    Rectangle sourceRec = {offsetX, offsetY, width, height};
    Rectangle destRec = {posX, posY, width, height};
    Vector2 origin = {0.0f, 0.0f};

    DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, WHITE);
  }
}
