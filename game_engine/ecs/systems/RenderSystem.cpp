#include "RenderSystem.hpp"
#include <cstddef>
#include "AssetManager.hpp"
#include "BackgroundTagComponent.hpp"
#include "ChatComponent.hpp"
#include "Client.hpp"
#include "Macro.hpp"
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
 * @brief Render all tracked entities and the chat UI when active.
 *
 * For each entity with a non-empty texture path, ensures the texture is loaded
 * and cached, initializes sprite animation frame dimensions if present and not
 * initialized, computes source and destination rectangles (respecting
 * SpriteComponent, RenderComponent, BackgroundTagComponent fullscreen-aspect
 * behavior, and ScaleComponent), and issues the draw call for the computed
 * rectangles. If a chat component exists and is active, renders the chat box,
 * messages, and input field.
 *
 * Observable side effects:
 * - Loads textures and stores them in the system's texture cache.
 * - Logs a warning and skips rendering if a texture fails to load or a
 *   background texture has zero height.
 *
 * @param deltaTime Time elapsed since the last update (seconds), used for any
 *                  time-based animation updates.
 */
void ecs::RenderSystem::update(float deltaTime) {
  std::set<Entity> entities;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    entities = _entities;
  }
  for (Entity entity : entities) {
    if (!_ecsManager.isEntityValid(entity))
      continue;
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

  if (_menuUI.getShowMenu() == true &&
      _client->getClientState() == client::ClientState::IN_CONNECTED_MENU) {
    _menuUI.drawMenu();
  }

  for (auto const &entity : _ecsManager.getAllEntities()) {
    if (_ecsManager.hasComponent<ecs::ChatComponent>(entity)) {
      _messagesUI.setChatEntity(entity);
      auto &chat = _ecsManager.getComponent<ecs::ChatComponent>(
          _messagesUI.getChatEntity().value());
      if (chat.isChatting) {
        _messagesUI.drawMessagesBox();
        _messagesUI.drawMessages();
        _messagesUI.drawMessageInputField(chat);
      }
      break;
    }
  }
}

/**
 * @brief Draws the chat messages background box on screen.
 *
 * Renders a translucent white rounded rectangle positioned near the bottom-left
 * of the screen to serve as the chat messages backdrop. The rectangle uses a
 * fixed size (two-thirds of the screen width by 365 pixels) and a subtle corner
 * radius.
 */
void ecs::ChatMessagesUI::drawMessagesBox() {
  Color rectColor = {255, 255, 255, 16};

  renderManager::Renderer::drawRectangleRounded(10, GetScreenHeight() - 415,
                                                (GetScreenWidth() / 3) * 2, 365,
                                                0.05f, rectColor);
}

/**
 * @brief Render the chat message history onto the screen when a client is
 * available.
 *
 * Collects chat messages from the connected client, formats each message with
 * an author prefix (omits the prefix for messages from "Server"), performs
 * word-wrapping to fit within the chat area using font metrics, and renders up
 * to the most recent CHAT_MAX_MESSAGES lines with their associated colors.
 *
 * The function does nothing if no client is connected.
 */
void ecs::ChatMessagesUI::drawMessages() {
  if (_client == nullptr)
    return;
  std::vector<client::ChatMessage> chatMessages = _client->getChatMessages();
  int lineHeight = chatUI::LINE_HEIGHT;
  int fontSize = chatUI::FONT_SIZE;
  int maxWidth = (GetScreenWidth() / 3) * 2 - chatUI::BOX_MAX_TEXT_LEN;
  Font font = GetFontDefault();
  std::vector<std::pair<std::string, Color>> allLines;

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
        allLines.push_back(
            {currentLine.substr(0, currentLine.size() - 1), chatMessage.color});
        currentLine = msg[i];
      }
    }
    if (!currentLine.empty())
      allLines.push_back({currentLine, chatMessage.color});
  }
  const int maxLines = client::CHAT_MAX_MESSAGES;
  size_t start_index = 0;
  if (allLines.size() > maxLines)
    start_index = allLines.size() - maxLines;
  int numVisibleLines = allLines.size() - start_index;
  int chatStartY = GetScreenHeight() - chatUI::BOX_BOTTOM_OFFSET -
                   (numVisibleLines - 1) * lineHeight;
  int y = chatStartY;

  for (size_t i = start_index; i < allLines.size(); ++i) {
    renderManager::Renderer::drawText(allLines[i].first.c_str(),
                                      chatUI::BOX_MAX_TEXT_LEN / 7, y, fontSize,
                                      allLines[i].second);
    y += lineHeight;
  }
}

/**
 * @brief Render the chat input field and the user's current input.
 *
 * Draws a translucent rounded rectangle at the bottom of the screen as the
 * input field and renders the chat's current message with a trailing cursor
 * underscore. If the message width exceeds the available input width, the
 * displayed text is truncated from the left so the rightmost characters (most
 * recent input) remain visible.
 *
 * @param chat ChatComponent whose `message` is displayed in the input field.
 */
void ecs::ChatMessagesUI::drawMessageInputField(const ChatComponent &chat) {
  Color rectColor = {255, 255, 255, 16};

  renderManager::Renderer::drawRectangleRounded(
      chatUI::INPUT_LEFT_OFFSET,
      GetScreenHeight() - chatUI::INPUT_BOTTOM_OFFSET,
      GetScreenWidth() - chatUI::INPUT_RIGHT_MARGIN, chatUI::INPUT_HEIGHT,
      chatUI::INPUT_ROUNDNESS, rectColor);
  std::string displayText = chat.message + "_";
  int maxInputWidth = GetScreenWidth() - chatUI::INPUT_TEXT_RIGHT_MARGIN;
  while (MeasureText(displayText.c_str(), 20) > maxInputWidth &&
         displayText.size() > 1)
    displayText = displayText.substr(1);
  renderManager::Renderer::drawText(
      displayText.c_str(), chatUI::LINE_HEIGHT,
      GetScreenHeight() - chatUI::INPUT_TEXT_Y_OFFSET, chatUI::FONT_SIZE,
      WHITE);
}

void ecs::MenuUI::loadTexture() {
  if (!_textureLoaded) {
    _startScreenTexture =
        asset::AssetManager::loadTexture(renderManager::START_SCREEN_PATH);
    if (_startScreenTexture.id != 0) {
      _textureLoaded = true;
    } else {
      TraceLog(LOG_WARNING, "MenuUI::loadTexture: failed to load %s",
               renderManager::START_SCREEN_PATH);
    }
  }
}

void ecs::MenuUI::drawMenuBackground() {
  loadTexture();
  if (!_textureLoaded)
    return;

  float screenWidth = GetScreenWidth();
  float scale = screenWidth / _startScreenTexture.width;

  Rectangle sourceRec = {0.0f, 0.0f, (float)_startScreenTexture.width,
                         (float)_startScreenTexture.height};
  Rectangle destRec = {0.0f, 0.0f, (float)_startScreenTexture.width * scale,
                       (float)_startScreenTexture.height * scale};
  Vector2 origin = {0.0f, 0.0f};
  DrawTexturePro(_startScreenTexture, sourceRec, destRec, origin, 0.0f, WHITE);
}

void ecs::MenuUI::drawMenu() {
  drawMenuBackground();
  drawStartButton();
}

void ecs::MenuUI::drawStartButton() {
  int posX = (GetScreenWidth() - menuUI::BUTTON_WIDTH) / 2;
  int posY = (GetScreenHeight() - menuUI::BUTTON_HEIGHT) / 2;
  Color buttonColor = DARKGRAY;

  renderManager::ButtonState startButton =
      renderManager::Renderer::handleButton(posX, posY, menuUI::BUTTON_WIDTH,
                                            menuUI::BUTTON_HEIGHT);
  switch (startButton) {
    case renderManager::ButtonState::HOVER:
      buttonColor = GRAY;
      break;
    case renderManager::ButtonState::CLICKED:
      buttonColor = LIGHTGRAY;
      break;
    case renderManager::ButtonState::RELEASED:
      _client->sendMatchmakingRequest();
      break;
    case renderManager::ButtonState::IDLE:
    default:
      buttonColor = DARKGRAY;
      break;
  }

  renderManager::Renderer::drawButton(posX, posY, menuUI::BUTTON_WIDTH,
                                      menuUI::BUTTON_HEIGHT, "Start Game",
                                      menuUI::FONT_SIZE, WHITE, buttonColor);
}
