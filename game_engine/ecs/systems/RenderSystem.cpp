#include "RenderSystem.hpp"
#include <cstddef>
#include "AssetManager.hpp"
#include "BackgroundTagComponent.hpp"
#include "ChatComponent.hpp"
#include "PositionComponent.hpp"
#include "RenderComponent.hpp"
#include "RenderManager.hpp"
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
    if (_ecsManager.hasComponent<ecs::ChatComponent>(entity)) {
      auto &chat = _ecsManager.getComponent<ecs::ChatComponent>(entity);
      if (chat.isChatting) {
        drawMessagesBox();
        drawMessages();
        drawMessageInputField(chat);
      }
    }
  }
}

void ecs::RenderSystem::drawMessagesBox() {
  Color rectColor = {255, 255, 255, 16};

  renderManager::Renderer::drawRectangleRounded(10, GetScreenHeight() - 415,
                                                (GetScreenWidth() / 3) * 2, 365,
                                                0.05f, rectColor);
}

void ecs::RenderSystem::drawMessages() {
  if (_client == nullptr)
    return;
  std::vector<client::ChatMessage> chatMessages = _client->getChatMessages();
  int lineHeight = 25;
  int fontSize = 20;
  int maxWidth = (GetScreenWidth() / 3) * 2 - 170;
  Font font = GetFontDefault();
  std::vector<std::string> allLines;

  for (const auto &chatMessage : chatMessages) {
    std::string msg;
    if (chatMessage.author == "Server")
      msg = chatMessage.message;
    else
      msg = "<" + chatMessage.author + "> " + chatMessage.message;
    std::string currentLine;
    for (size_t i = 0; i < msg.size(); ++i) {
      currentLine += msg[i];
      int width = MeasureTextEx(font, currentLine.c_str(), fontSize, 0).x;
      if (width > maxWidth && currentLine.size() > 1) {
        allLines.push_back(currentLine.substr(0, currentLine.size() - 1));
        currentLine = msg[i];
      }
    }
    if (!currentLine.empty())
      allLines.push_back(currentLine);
  }
  const int maxLines = 14;
  size_t start_index = 0;
  if (allLines.size() > maxLines)
    start_index = allLines.size() - maxLines;
  int numVisibleLines = allLines.size() - start_index;
  int chatStartY = GetScreenHeight() - 80 - (numVisibleLines - 1) * lineHeight;
  int y = chatStartY;

  for (size_t i = start_index; i < allLines.size(); ++i) {
    auto msg =
        chatMessages[i < chatMessages.size() ? i : chatMessages.size() - 1];
    renderManager::Renderer::drawText(allLines[i].c_str(), 25, y, fontSize,
                                      msg.color);
    y += lineHeight;
  }
}

void ecs::RenderSystem::drawMessageInputField(const ChatComponent &chat) {
  Color rectColor = {255, 255, 255, 16};

  renderManager::Renderer::drawRectangleRounded(
      10, GetScreenHeight() - 40, GetScreenWidth() - 20, 30, 0.5f, rectColor);
  renderManager::Renderer::drawText((chat.message + "_").c_str(), 25,
                                    GetScreenHeight() - 35, 20, WHITE);
}
