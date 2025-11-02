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
    _menuUI.handleInput();
    _menuUI.drawMenu();
  }

  if (_menuUI.isWaitingForChallenge() &&
      _client->getChallenge().isChallengeReceived()) {
    try {
      uint32_t roomId = _menuUI.getRoomId();
      if (roomId != (uint32_t)-1) {
        _client->sendJoinRoom(roomId, _menuUI.getPassword());
      }
    } catch (const std::exception &e) {
      TraceLog(LOG_WARNING, "Invalid room ID format.");
    }
    _menuUI.setWaitingForChallenge(false);
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

uint32_t ecs::MenuUI::getRoomId() const {
  try {
    return std::stoul(_roomIdJoinInput);
  } catch (const std::exception &e) {
    return -1;
  }
}

void ecs::MenuUI::handleInput() {
  if (_activeField != NONE) {
    int key = GetCharPressed();
    while (key > 0) {
      if ((key >= 32) && (key <= 125)) {
        switch (_activeField) {
        case ROOM_NAME_CREATE:
          _roomNameCreateInput += (char)key;
          break;
        case PASSWORD_CREATE:
          _passwordCreateInput += (char)key;
          break;
        case ROOM_ID_JOIN:
          _roomIdJoinInput += (char)key;
          break;
        case PASSWORD_JOIN:
          _passwordJoinInput += (char)key;
          break;
        default:
          break;
        }
      }
      key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
      switch (_activeField) {
      case ROOM_NAME_CREATE:
        if (!_roomNameCreateInput.empty()) _roomNameCreateInput.pop_back();
        break;
      case PASSWORD_CREATE:
        if (!_passwordCreateInput.empty()) _passwordCreateInput.pop_back();
        break;
      case ROOM_ID_JOIN:
        if (!_roomIdJoinInput.empty()) _roomIdJoinInput.pop_back();
        break;
      case PASSWORD_JOIN:
        if (!_passwordJoinInput.empty()) _passwordJoinInput.pop_back();
        break;
      default:
        break;
      }
    }
  }
}

void ecs::MenuUI::drawMenu() {
  drawMenuBackground();
  switch (_menuState) {
  case MAIN:
    drawMainMenu();
    break;
  case CREATE_ROOM:
    drawCreateRoomMenu();
    break;
  case JOIN_ROOM:
    drawJoinRoomMenu();
    break;
  }
}

void ecs::MenuUI::drawMainMenu() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Matchmaking Button
    int startButtonX = (screenWidth - menuUI::BUTTON_WIDTH) / 2;
    int startButtonY = (screenHeight - menuUI::BUTTON_HEIGHT) / 2 - menuUI::BUTTON_HEIGHT - 10;
    Color startButtonColor = DARKGRAY;
    renderManager::ButtonState startButtonState = renderManager::Renderer::handleButton(startButtonX, startButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT);
    if (startButtonState == renderManager::ButtonState::HOVER) startButtonColor = GRAY;
    if (startButtonState == renderManager::ButtonState::CLICKED) startButtonColor = LIGHTGRAY;
    if (startButtonState == renderManager::ButtonState::RELEASED) _client->sendMatchmakingRequest();
    renderManager::Renderer::drawButton(startButtonX, startButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT, "Matchmaking", menuUI::FONT_SIZE, WHITE, startButtonColor);

    // Create Room Button
    int createButtonY = startButtonY + menuUI::BUTTON_HEIGHT + 10;
    Color createButtonColor = DARKGRAY;
    renderManager::ButtonState createButtonState = renderManager::Renderer::handleButton(startButtonX, createButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT);
    if (createButtonState == renderManager::ButtonState::HOVER) createButtonColor = GRAY;
    if (createButtonState == renderManager::ButtonState::CLICKED) createButtonColor = LIGHTGRAY;
    if (createButtonState == renderManager::ButtonState::RELEASED) _menuState = CREATE_ROOM;
    renderManager::Renderer::drawButton(startButtonX, createButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT, "Create Room", menuUI::FONT_SIZE, WHITE, createButtonColor);

    // Join Room Button
    int joinButtonY = createButtonY + menuUI::BUTTON_HEIGHT + 10;
    Color joinButtonColor = DARKGRAY;
    renderManager::ButtonState joinButtonState = renderManager::Renderer::handleButton(startButtonX, joinButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT);
    if (joinButtonState == renderManager::ButtonState::HOVER) joinButtonColor = GRAY;
    if (joinButtonState == renderManager::ButtonState::CLICKED) joinButtonColor = LIGHTGRAY;
    if (joinButtonState == renderManager::ButtonState::RELEASED) _menuState = JOIN_ROOM;
    renderManager::Renderer::drawButton(startButtonX, joinButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT, "Join Room", menuUI::FONT_SIZE, WHITE, joinButtonColor);
}

void ecs::MenuUI::drawInputField(const char *label, Rectangle bounds, std::string &text, ActiveField field, bool isPassword) {
    renderManager::Renderer::drawText(label, bounds.x, bounds.y - 25, 20, WHITE);
    if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            _activeField = field;
        }
    }
    
    if (_activeField == field) {
        renderManager::Renderer::drawRectangle(bounds.x - 2, bounds.y - 2, bounds.width + 4, bounds.height + 4, RED);
    } else {
        renderManager::Renderer::drawRectangle(bounds.x - 1, bounds.y - 1, bounds.width + 2, bounds.height + 2, DARKGRAY);
    }

    renderManager::Renderer::drawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, LIGHTGRAY);

    std::string displayedText = isPassword ? std::string(text.length(), '*') : text;
    if (_activeField == field) {
        if (static_cast<int>(GetTime() * 2) % 2 == 0) {
            displayedText += '_';
        }
    }
    
    while (MeasureText(displayedText.c_str(), 20) > bounds.width - 10) {
        if (isPassword) {
            break;
        }
        displayedText.erase(0, 1);
    }

    renderManager::Renderer::drawText(displayedText.c_str(), bounds.x + 5, bounds.y + 10, 20, BLACK);
}

void ecs::MenuUI::drawCreateRoomMenu() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    Rectangle nameBounds = {static_cast<float>((screenWidth - 300) / 2), static_cast<float>((screenHeight - 100) / 2 - 40), 300, 40};
    drawInputField("Room Name:", nameBounds, _roomNameCreateInput, ROOM_NAME_CREATE, false);

    Rectangle passBounds = {static_cast<float>((screenWidth - 300) / 2), static_cast<float>((screenHeight - 100) / 2 + 50), 300, 40};
    drawInputField("Password:", passBounds, _passwordCreateInput, PASSWORD_CREATE, true);

    // Create Button
    int createButtonX = (screenWidth - menuUI::BUTTON_WIDTH) / 2;
    int createButtonY = passBounds.y + passBounds.height + 20;
    Color createButtonColor = DARKGRAY;
    renderManager::ButtonState createButtonState = renderManager::Renderer::handleButton(createButtonX, createButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT);
    if (createButtonState == renderManager::ButtonState::HOVER) createButtonColor = GRAY;
    if (createButtonState == renderManager::ButtonState::CLICKED) createButtonColor = LIGHTGRAY;
    if (createButtonState == renderManager::ButtonState::RELEASED) {
        _client->createRoom(_roomNameCreateInput, _passwordCreateInput);
        _menuState = MAIN;
    }
    renderManager::Renderer::drawButton(createButtonX, createButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT, "Create", menuUI::FONT_SIZE, WHITE, createButtonColor);

    // Back Button
    int backButtonY = createButtonY + menuUI::BUTTON_HEIGHT + 10;
    Color backButtonColor = DARKGRAY;
    renderManager::ButtonState backButtonState = renderManager::Renderer::handleButton(createButtonX, backButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT);
    if (backButtonState == renderManager::ButtonState::HOVER) backButtonColor = GRAY;
    if (backButtonState == renderManager::ButtonState::CLICKED) backButtonColor = LIGHTGRAY;
    if (backButtonState == renderManager::ButtonState::RELEASED) _menuState = MAIN;
    renderManager::Renderer::drawButton(createButtonX, backButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT, "Back", menuUI::FONT_SIZE, WHITE, backButtonColor);
}

void ecs::MenuUI::drawJoinRoomMenu() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    Rectangle idBounds = {static_cast<float>((screenWidth - 300) / 2), static_cast<float>((screenHeight - 100) / 2 - 40), 300, 40};
    drawInputField("Room ID:", idBounds, _roomIdJoinInput, ROOM_ID_JOIN, false);

    Rectangle passBounds = {static_cast<float>((screenWidth - 300) / 2), static_cast<float>((screenHeight - 100) / 2 + 50), 300, 40};
    drawInputField("Password:", passBounds, _passwordJoinInput, PASSWORD_JOIN, true);

    // Join Button
    int joinButtonX = (screenWidth - menuUI::BUTTON_WIDTH) / 2;
    int joinButtonY = passBounds.y + passBounds.height + 20;
    Color joinButtonColor = DARKGRAY;
    renderManager::ButtonState joinButtonState = renderManager::Renderer::handleButton(joinButtonX, joinButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT);
    if (joinButtonState == renderManager::ButtonState::HOVER) joinButtonColor = GRAY;
    if (joinButtonState == renderManager::ButtonState::CLICKED) joinButtonColor = LIGHTGRAY;
    if (joinButtonState == renderManager::ButtonState::RELEASED) {
        try {
            uint32_t roomId = getRoomId();
            if (roomId != (uint32_t)-1) {
                _client->sendRequestChallenge(roomId);
                setWaitingForChallenge(true);
            }
        } catch (const std::exception &e) {
            TraceLog(LOG_WARNING, "Invalid room ID format.");
        }
    }
    renderManager::Renderer::drawButton(joinButtonX, joinButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT, "Join", menuUI::FONT_SIZE, WHITE, joinButtonColor);

    // Back Button
    int backButtonY = joinButtonY + menuUI::BUTTON_HEIGHT + 10;
    Color backButtonColor = DARKGRAY;
    renderManager::ButtonState backButtonState = renderManager::Renderer::handleButton(joinButtonX, backButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT);
    if (backButtonState == renderManager::ButtonState::HOVER) backButtonColor = GRAY;
    if (backButtonState == renderManager::ButtonState::CLICKED) backButtonColor = LIGHTGRAY;
    if (backButtonState == renderManager::ButtonState::RELEASED) _menuState = MAIN;
    renderManager::Renderer::drawButton(joinButtonX, backButtonY, menuUI::BUTTON_WIDTH, menuUI::BUTTON_HEIGHT, "Back", menuUI::FONT_SIZE, WHITE, backButtonColor);
}
