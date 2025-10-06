#include "BackgroundSystem.hpp"
#include <iostream>
#include <vector>
#include "PositionComponent.hpp"
#include "RenderManager.hpp"
#include "raylib.h"

ecs::BackgroundSystem::~BackgroundSystem() noexcept {
  for (auto &pair : _textureCache)
    UnloadTexture(pair.second);
}

void ecs::BackgroundSystem::update(float deltaTime) {
  (void)deltaTime;

  if (_entities.size() != 2)
    return;
  const std::string &path = renderManager::BG_PATH;
  if (_textureCache.find(path) == _textureCache.end()) {
    Texture2D newTexture = LoadTexture(path.c_str());

    if (newTexture.id == 0)
      return;
    _textureCache[path] = newTexture;
  }
  Texture2D &texture = _textureCache[path];
  float screenHeight = GetScreenHeight();
  float aspectRatio = static_cast<float>(texture.width) / static_cast<float>(texture.height);
  float scaledWidth = screenHeight * aspectRatio;

  std::vector<Entity> entities(_entities.begin(), _entities.end());
  auto &pos1 = _ecsManager.getComponent<PositionComponent>(entities[0]);
  auto &pos2 = _ecsManager.getComponent<PositionComponent>(entities[1]);

  if (pos1.x <= -scaledWidth)
    pos1.x = pos2.x + scaledWidth;
  else if (pos2.x <= -scaledWidth)
    pos2.x = pos1.x + scaledWidth;
}
