#include "InputSystem.hpp"
#include <cmath>
#include "ChatComponent.hpp"
#include "Client.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PositionComponent.hpp"
#include "RaylibUtils.hpp"
#include "SpriteAnimationComponent.hpp"
#include "raylib.h"

namespace ecs {
  /**
   * @brief Tests whether the specified key was pressed according to AZERTY
   * keyboard mapping.
   *
   * @param key Key to test.
   * @return true if the key was pressed, false otherwise.
   */
  static bool IsKeyPressedAZERTY(KeyboardKey key) {
    return utils::Raylib::IsKeyPressedAZERTY(key);
  }

  /**
   * @brief Checks whether the given keyboard key is currently held down using
   * AZERTY layout semantics.
   *
   * @param key The key to test.
   * @return bool `true` if the specified key is currently down, `false`
   * otherwise.
   */
  static bool IsKeyDownAZERTY(KeyboardKey key) {
    return utils::Raylib::IsKeyDownAZERTY(key);
  }

  /**
   * @brief Handle player and UI keyboard input: matchmaking, chat UI, movement,
   * vertical sprite animation, and shooting.
   *
   * Processes client state and input:
   * - In the connected menu, pressing 'M' sends a matchmaking request.
   * - In game or room-waiting states, iterates entities with a
   * SpriteAnimationComponent to:
   *   - send a movement input bitmask when arrow keys are pressed,
   *   - update the sprite's vertical animation state (UP plays forward from the
   * neutral frame, DOWN plays backward from the neutral frame, neither or both
   * stops and resets to the neutral frame with non-negative frameTime),
   *   - send a shoot request with the entity's position when SPACE is pressed.
   *
   * @param deltaTime Time elapsed since the last update in seconds (unused).
   */
  void InputSystem::update([[maybe_unused]] float deltaTime) {
    if (_client == nullptr)
      return;

    client::ClientState clientState = _client->getClientState();

    if (clientState == client::ClientState::IN_CONNECTED_MENU) {
      if (IsKeyPressedAZERTY(KEY_M)) {
        _client->sendMatchmakingRequest();
        TraceLog(LOG_INFO,
                 "[INPUT SYSTEM] M pressed - sending matchmaking request");
      }
      return;
    }

    if (clientState != client::ClientState::IN_GAME &&
        clientState != client::ClientState::IN_ROOM_WAITING) {
      return;
    }

    if (loadUIEntities())
      return;

    for (auto const &entity : _entities) {
      if (!_ecsManager.hasComponent<SpriteAnimationComponent>(entity)) {
        continue;
      }
      auto &animation =
          _ecsManager.getComponent<SpriteAnimationComponent>(entity);

      bool upPressed = IsKeyDown(KEY_UP);
      bool downPressed = IsKeyDown(KEY_DOWN);
      bool leftPressed = IsKeyDown(KEY_LEFT);
      bool rightPressed = IsKeyDown(KEY_RIGHT);

      std::uint8_t inputs = 0;
      if (upPressed)
        inputs |= static_cast<std::uint8_t>(MovementInputType::UP);
      if (downPressed)
        inputs |= static_cast<std::uint8_t>(MovementInputType::DOWN);
      if (leftPressed)
        inputs |= static_cast<std::uint8_t>(MovementInputType::LEFT);
      if (rightPressed)
        inputs |= static_cast<std::uint8_t>(MovementInputType::RIGHT);

      if (inputs != 0 && _client != nullptr)
        _client->sendInput(inputs);

      if (upPressed && !downPressed) {
        if (animation.frameTime < 0 || !animation.isPlaying) {
          animation.currentFrame = animation.neutralFrame;
          animation.frameTime = std::abs(animation.frameTime);
          animation.isPlaying = true;
        }
      } else if (downPressed && !upPressed) {
        if (animation.frameTime > 0 || !animation.isPlaying) {
          animation.currentFrame = animation.neutralFrame;
          animation.frameTime = -std::abs(animation.frameTime);
          animation.isPlaying = true;
        }
      } else {
        animation.isPlaying = false;
        animation.currentFrame = animation.neutralFrame;
        animation.frameTime = std::abs(animation.frameTime);
      }

      if (IsKeyPressed(KEY_SPACE) && _client != nullptr) {
        auto &position = _ecsManager.getComponent<PositionComponent>(entity);
        _client->sendShoot(position.x, position.y);
      }
    }
  }

  /**
   * @brief Locates a UI entity with a ChatComponent and processes chat input
   * and state.
   *
   * Searches all entities for the first ChatComponent. If found, updates its
   * chat state: when chatting, appends valid typed characters, handles
   * backspace (including repeat), and on Enter sends the accumulated message
   * via the client (if available) and clears it. While not chatting, pressing
   * AZERTY T enters chat mode; pressing Escape while chatting exits chat mode
   * and clears the message. Adjusts the global exit key to disable it while
   * chatting and re-enable Escape when not.
   *
   * @return true if chat input handling consumed the frame (e.g., message was
   * sent or chat mode was entered), `false` otherwise.
   */
  bool InputSystem::loadUIEntities() {
    Entity uiEntity = INVALID_ENTITY;
    for (auto const &entity : _ecsManager.getAllEntities()) {
      if (_ecsManager.hasComponent<ChatComponent>(entity)) {
        uiEntity = entity;
        break;
      }
    }

    if (uiEntity != INVALID_ENTITY) {
      auto &chat = _ecsManager.getComponent<ChatComponent>(uiEntity);

      if (chat.isChatting) {
        char character = static_cast<char>(GetCharPressed());
        if (character != ASCII_NULL && character >= ASCII_SPACE &&
            character != ASCII_DEL)
          chat.message += character;
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
          if (!chat.message.empty())
            chat.message.pop_back();
        }
        if (IsKeyPressed(KEY_ENTER)) {
          if (!chat.message.empty() && _client != nullptr)
            _client->sendChatMessage(chat.message);
          chat.message.clear();
          return true;
        }
      }

      if (IsKeyPressedAZERTY(KEY_T) && !chat.isChatting) {
        chat.isChatting = true;
        return true;
      } else if (IsKeyPressedAZERTY(KEY_ESCAPE) && chat.isChatting) {
        chat.isChatting = false;
        chat.message.clear();
        return false;
      }

      if (chat.isChatting)
        SetExitKey(KEY_NULL);
      else if (!chat.isChatting)
        SetExitKey(KEY_ESCAPE);

      return chat.isChatting;
    }
    return false;
  }
}  // namespace ecs