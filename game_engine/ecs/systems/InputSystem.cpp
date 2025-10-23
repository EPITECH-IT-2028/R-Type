#include "InputSystem.hpp"
#include <cmath>
#include "Client.hpp"
#include "Packet.hpp"
#include "PositionComponent.hpp"
#include "RaylibUtils.hpp"
#include "SpriteAnimationComponent.hpp"
#include "raylib.h"

namespace ecs {
  /**
   * @brief Tests whether the specified key was pressed according to AZERTY keyboard mapping.
   *
   * @param key Key to test.
   * @return true if the key was pressed, false otherwise.
   */
  static bool IsKeyPressedAZERTY(KeyboardKey key) {
    return utils::Raylib::IsKeyPressedAZERTY(key);
  }

  /**
   * @brief Checks whether the given keyboard key is currently held down using AZERTY layout semantics.
   *
   * @param key The key to test.
   * @return bool `true` if the specified key is currently down, `false` otherwise.
   */
  static bool IsKeyDownAZERTY(KeyboardKey key) {
    return utils::Raylib::IsKeyDownAZERTY(key);
  }

  /**
   * @brief Process keyboard input for matchmaking, player movement, shooting,
   * and update per-entity vertical sprite animation state.
   *
   * When a client is present, handles:
   * - CONNECTED_MENU: pressing 'M' sends a matchmaking request.
   * - IN_GAME or IN_ROOM_WAITING: for each entity with a
   * SpriteAnimationComponent, sends a movement input bitmask if any directional
   * keys are pressed, updates the sprite's vertical animation state (UP plays
   * forward from the neutral frame, DOWN plays backward from the neutral frame,
   * neither or both stops and resets to the neutral frame with non-negative
   * frameTime), and sends a shoot request with the entity's position when SPACE
   * is pressed.
   *
   * @param deltaTime Time elapsed since the last update in seconds (provided by
   * caller; not used by this implementation).
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

    if (_isChatting) {
      char character = static_cast<char>(GetCharPressed());
      if (character != 0) {
        _chatMessage += character;
        TraceLog(LOG_INFO, "[CHAT] Current message: %s", _chatMessage.c_str());
      }
      if (IsKeyPressed(KEY_BACKSPACE)) {
        if (!_chatMessage.empty())
          _chatMessage.pop_back();
      }
      if (IsKeyPressed(KEY_ENTER)) {
        if (!_chatMessage.empty() && _client != nullptr)
          TraceLog(LOG_INFO, "[CHAT] Sending message: %s",
                   _chatMessage.c_str());
        _chatMessage.clear();
        _isChatting = false;
        TraceLog(LOG_DEBUG, "[CHAT] Exiting chatting mode");
        return;
      }
    }

    if (IsKeyPressed(KEY_ENTER)) {
      TraceLog(LOG_DEBUG, "[CHAT] Enter pressed - entering chatting mode");
      _isChatting = true;
      return;
    }

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
}  // namespace ecs
