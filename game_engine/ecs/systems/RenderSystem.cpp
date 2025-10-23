#include "RenderSystem.hpp"
#include "AssetManager.hpp"
#include "BackgroundTagComponent.hpp"
#include "ChatComponent.hpp"
#include "Macro.hpp"
#include "PositionComponent.hpp"
#include "RenderComponent.hpp"
#include "ScaleComponent.hpp"
#include "SpriteComponent.hpp"
#include "raylib.h"

/**
 * @brief Releases GPU texture resources held by the render system.
 *
 * Iterates over the internal texture cache and unloads each cached Texture2D to
 * free associated GPU memory.
 */
ecs::RenderSystem::~RenderSystem() noexcept {
  for (auto &pair : _textureCache) {
    UnloadTexture(pair.second);
  }
}

#include "SpriteAnimationComponent.hpp"

/**
 * @brief Renders all entities managed by this system, loading and caching
 * textures and initializing sprite animations when needed.
 *
 * Iterates over tracked entities and, for each one with a non-empty texture
 * path, ensures the texture is loaded and cached, initializes
 * SpriteAnimationComponent frame dimensions if the component is present and not
 * initialized, constructs source and destination rectangles (respecting
 * SpriteComponent source rects, RenderComponent size/offsets,
 * BackgroundTagComponent fullscreen-aspect behavior, and ScaleComponent), and
 * issues the draw call for the computed rectangles.
 *
 * Observable side effects:
 * - Loads textures from disk and stores them in the system's texture cache.
 * - Logs a warning and skips rendering if a texture fails to load or has zero
 * height for background entities.
 */
void ecs::RenderSystem::update(float deltaTime) {
  for (Entity entity : _entities) {
    auto &positionComp =
        _ecsManager.getComponent<ecs::PositionComponent>(entity);
    auto &renderComp = _ecsManager.getComponent<ecs::RenderComponent>(entity);
    const std::string &path = renderComp._texturePath;

    if (path.empty())
      continue;

    if (_textureCache.find(path) == _textureCache.end()) {
      Texture2D newTexture = asset::AssetManager::loadTexture(path);
      if (newTexture.id == 0) {
        TraceLog(LOG_WARNING, "RenderSystem::update: échec du chargement de %s",
                 path.c_str());
        continue;
      }
      _textureCache[path] = newTexture;
    }
    Texture2D &texture = _textureCache[path];

    if (_ecsManager.hasComponent<ecs::SpriteAnimationComponent>(entity)) {
      auto &anim =
          _ecsManager.getComponent<ecs::SpriteAnimationComponent>(entity);
      if (!anim.isInitialized) {
        if (anim.totalColumns > 0 && anim.totalRows > 0) {
          anim.frameWidth = texture.width / anim.totalColumns;
          anim.frameHeight = texture.height / anim.totalRows;
          anim.isInitialized = true;
        }
      }
    }

    Rectangle sourceRec = {0.0f, 0.0f, static_cast<float>(texture.width),
                           static_cast<float>(texture.height)};
    if (_ecsManager.hasComponent<ecs::SpriteComponent>(entity)) {
      auto &spriteComp = _ecsManager.getComponent<ecs::SpriteComponent>(entity);
      sourceRec = spriteComp.sourceRect;
    }
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
      destRec = {positionComp.x, positionComp.y, destWidth, destHeight};
    } else {
      destRec.x = positionComp.x + renderComp._offsetX;
      destRec.y = positionComp.y + renderComp._offsetY;
      destRec.width = (renderComp._width > 0)
                          ? renderComp._width
                          : static_cast<float>(sourceRec.width);
      destRec.height = (renderComp._height > 0)
                           ? renderComp._height
                           : static_cast<float>(sourceRec.height);
    }
    if (_ecsManager.hasComponent<ecs::ScaleComponent>(entity)) {
      auto &scaleComp = _ecsManager.getComponent<ecs::ScaleComponent>(entity);
      destRec.width *= scaleComp.scaleX;
      destRec.height *= scaleComp.scaleY;
    }
    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, WHITE);
  }

  for (auto const &entity : _ecsManager.getAllEntities()) {
    if (_ecsManager.hasComponent<ChatComponent>(entity)) {
      auto &chat = _ecsManager.getComponent<ChatComponent>(entity);
      if (chat.isChatting)
        DrawText((chat.playerName + ": " + chat.message + "_").c_str(), 10,
                 WINDOW_HEIGHT - 20, 20, WHITE);
    }
  }
}
