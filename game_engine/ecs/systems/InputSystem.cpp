#include "InputSystem.hpp"
#include <cmath>
#include "Client.hpp"
#include "Packet.hpp"
#include "PositionComponent.hpp"
#include "SpeedComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "VelocityComponent.hpp"
#include "raylib.h"

namespace ecs {
  /**
   * @brief Update per-entity movement velocity and vertical sprite animation
   * state based on keyboard input.
   *
   * For each entity in the system, reads up/down/left/right key state to
   * compute a normalized movement direction, sends input to the client, and
   * updates the entity's SpriteAnimationComponent vertical state:
   * - When UP (and not DOWN) is pressed, ensures animation plays forward
   * (positive frameTime) from the neutral frame.
   * - When DOWN (and not UP) is pressed, ensures animation plays backward
   * (negative frameTime) from the neutral frame.
   * - When neither or both vertical keys are pressed, stops the animation and
   * resets to the neutral frame with non-negative frameTime.
   *
   * Also handles shooting input: when the space bar is pressed, calls the
   * client's sendShoot method with the local player's position.
   *
   * @param deltaTime Time elapsed since the last update in seconds (provided by
   * caller; not used by this implementation).
   */
  void InputSystem::update([[maybe_unused]] float deltaTime) {
    if (_client == nullptr)
      return;

    client::ClientState clientState = _client->getClientState();

    if (clientState == client::ClientState::CONNECTED_MENU) {
      if (IsKeyPressed(KEY_M)) {
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

    for (auto const &entity : _entities) {
      if (!_ecsManager.hasComponent<VelocityComponent>(entity) ||
          !_ecsManager.hasComponent<SpeedComponent>(entity) ||
          !_ecsManager.hasComponent<SpriteAnimationComponent>(entity)) {
        continue;
      }
      auto &velocity = _ecsManager.getComponent<VelocityComponent>(entity);
      auto const &speed = _ecsManager.getComponent<SpeedComponent>(entity);
      auto &animation =
          _ecsManager.getComponent<SpriteAnimationComponent>(entity);

      bool upPressed = IsKeyDown(KEY_UP);
      bool downPressed = IsKeyDown(KEY_DOWN);
      bool leftPressed = IsKeyDown(KEY_LEFT);
      bool rightPressed = IsKeyDown(KEY_RIGHT);

      uint8_t inputs = 0;
      if (upPressed)
        inputs |= static_cast<uint8_t>(MovementInputType::UP);
      if (downPressed)
        inputs |= static_cast<uint8_t>(MovementInputType::DOWN);
      if (leftPressed)
        inputs |= static_cast<uint8_t>(MovementInputType::LEFT);
      if (rightPressed)
        inputs |= static_cast<uint8_t>(MovementInputType::RIGHT);

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
        TraceLog(
            LOG_INFO,
            "[INPUT SYSTEM] Space pressed - shooting from position (%f, %f)",
            position.x, position.y);
      }
    }
  }
}  // namespace ecs
