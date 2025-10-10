#include "BackgroundSystem.hpp"
#include "PositionComponent.hpp"
#include "RenderManager.hpp"
#include "raylib.h"

ecs::BackgroundSystem::~BackgroundSystem() noexcept {
  for (auto &pair : _textureCache)
    UnloadTexture(pair.second);
}

void ecs::BackgroundSystem::update(float deltaTime) {
  (void)deltaTime;

  if (_entities.empty()) {
    return;
  }

  const std::string &path = renderManager::BG_PATH;
  if (_textureCache.find(path) == _textureCache.end()) {
    Texture2D newTexture = LoadTexture(path.c_str());
    if (newTexture.id == 0)
      return;
    _textureCache[path] = newTexture;
  }
  Texture2D &texture = _textureCache[path];

  if (texture.height <= 0) {
    TraceLog(LOG_WARNING,
             "BackgroundSystem::update: Texture height is zero for path %s",
             path.c_str());
    return;
  }

  float screenHeight = GetScreenHeight();
  float aspectRatio =
      static_cast<float>(texture.width) / static_cast<float>(texture.height);
  float scaledWidth = screenHeight * aspectRatio;
  float totalWidth = scaledWidth * _entities.size();

  for (auto const &entity : _entities) {
    auto &position = _ecsManager.getComponent<PositionComponent>(entity);

    if (position.x <= -scaledWidth) {
      position.x += totalWidth;
    }
  }
}
