#include "ServerInputSystem.hpp"
#include "Events.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "SpeedComponent.hpp"
#include <cmath>
#include <algorithm>

void ecs::ServerInputSystem::update(float deltaTime) {
  if (_pendingInputs.empty() || !_eventQueue)
    return;
  for (auto &[entityId, inputs] : _pendingInputs) {
    if (inputs.empty())
      continue;
    if (!_ecsManagerPtr ||
        !_ecsManagerPtr->hasComponent<PositionComponent>(entityId) ||
        !_ecsManagerPtr->hasComponent<SpeedComponent>(entityId))
      continue;
    processInput(entityId, inputs, deltaTime);
    sendPositionUpdate(entityId);
  }
  _pendingInputs.clear();
}

void ecs::ServerInputSystem::queueInput(Entity entityId,
                                        const PlayerInput &input) {
  _pendingInputs[entityId].push_back(input);
}

void ecs::ServerInputSystem::processInput(
    Entity entityId, const std::vector<PlayerInput> &inputs, float deltaTime) {
  float deltaX = 0.0f;
  float deltaY = 0.0f;
  auto &position = _ecsManagerPtr->getComponent<PositionComponent>(entityId);
  const auto &speed = _ecsManagerPtr->getComponent<SpeedComponent>(entityId);
  float moveDistance = speed.speed * deltaTime;

  for (const auto &input : inputs) {
    switch (input.input) {
      case MovementInputType::UP:
        deltaY -= moveDistance;
        break;
      case MovementInputType::DOWN:
        deltaY += moveDistance;
        break;
      case MovementInputType::LEFT:
        deltaX -= moveDistance;
        break;
      case MovementInputType::RIGHT:
        deltaX += moveDistance;
        break;
    }
  }

  float length = std::sqrt(deltaX * deltaX + deltaY * deltaY);
  if (length > moveDistance) {
    deltaX = (deltaX / length) * moveDistance;
    deltaY = (deltaY / length) * moveDistance;
  }

  position.x += deltaX;
  position.y += deltaY;

  position.x = std::clamp(position.x, 0.0f, static_cast<float>(WINDOW_WIDTH));
  position.y = std::clamp(position.y, 0.0f, static_cast<float>(WINDOW_HEIGHT));
}

void ecs::ServerInputSystem::sendPositionUpdate(Entity entityId) {
  if (!_ecsManagerPtr->hasComponent<PositionComponent>(entityId) ||
      !_ecsManagerPtr->hasComponent<PlayerComponent>(entityId))
    return;

  const auto &position =
      _ecsManagerPtr->getComponent<PositionComponent>(entityId);
  const auto &player = _ecsManagerPtr->getComponent<PlayerComponent>(entityId);

  queue::PositionEvent positionEvent;
  positionEvent.player_id = player.player_id;
  positionEvent.x = position.x;
  positionEvent.y = position.y;
  positionEvent.sequence_number = 0;
  _eventQueue->addRequest(positionEvent);
}
