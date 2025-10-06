#include "RenderSystem.hpp"
#include "BackgroundTagComponent.hpp"
#include "PositionComponent.hpp"
#include "RenderComponent.hpp"
#include "raylib.h"

ecs::RenderSystem::~RenderSystem() noexcept {
  for (auto &pair : _textureCache) {
    UnloadTexture(pair.second);
  }
}

void ecs::RenderSystem::update(float deltaTime) {
  for (Entity entity : _entities) {
    auto &renderComp = _ecsManager.getComponent<ecs::RenderComponent>(entity);
    const std::string &path = renderComp._texturePath;

    if (path.empty())
      continue;

    if (_textureCache.find(path) == _textureCache.end()) {
      Texture2D newTexture = LoadTexture(path.c_str());

      if (newTexture.id == 0)
        continue;
      _textureCache[path] = newTexture;
    }

    Texture2D &texture = _textureCache[path];
    auto &position = _ecsManager.getComponent<ecs::PositionComponent>(entity);

    Rectangle sourceRec = {0.0f, 0.0f, static_cast<float>(texture.width),
                           static_cast<float>(texture.height)};
    Rectangle destRec;

    if (_ecsManager.hasComponent<BackgroundTagComponent>(entity)) {
      if (texture.height <= 0) {
        TraceLog(LOG_WARNING,
                 "RenderSystem::update: Texture height is zero for path %s",
                 path.c_str());
        continue;
      }
      float screenHeight = GetScreenHeight();
      float sourceAspectRatio = static_cast<float>(texture.width) /
                                static_cast<float>(texture.height);
      float destHeight = screenHeight;
      float destWidth = destHeight * sourceAspectRatio;
      destRec = {position.x, position.y, destWidth, destHeight};
    } else {
      destRec.x = position.x + renderComp._offsetX;
      destRec.y = position.y + renderComp._offsetY;
      destRec.width = (renderComp._width > 0)
                          ? renderComp._width
                          : static_cast<float>(texture.width);
      destRec.height = (renderComp._height > 0)
                           ? renderComp._height
                           : static_cast<float>(texture.height);
    }

    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, WHITE);
  }
}
