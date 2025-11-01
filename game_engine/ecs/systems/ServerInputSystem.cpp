#include "ServerInputSystem.hpp"
#include <algorithm>
#include <cmath>
#include "Events.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "SpeedComponent.hpp"

void ecs::ServerInputSystem::update(float deltaTime) {
  std::unordered_map<Entity, std::vector<PlayerInput>> inputsToProcess;
  {
    std::lock_guard<std::mutex> lock(_inputMutex);
    if (_pendingInputs.empty())
      return;
    inputsToProcess.swap(_pendingInputs);
  }

  if (inputsToProcess.empty() || !_eventQueue)
    return;

  for (auto &[entityId, inputs] : inputsToProcess) {
    if (inputs.empty())
      continue;
    if (!_ecsManagerPtr ||
        !_ecsManagerPtr->hasComponent<PositionComponent>(entityId) ||
        !_ecsManagerPtr->hasComponent<SpeedComponent>(entityId))
      continue;
    processInput(entityId, inputs, deltaTime);
    sendPositionUpdate(entityId);
  }
}

void ecs::ServerInputSystem::queueInput(Entity entityId,
                                        const PlayerInput &input) {
  std::lock_guard<std::mutex> lock(_inputMutex);
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
    uint8_t val = static_cast<uint8_t>(input.input);

    if (val & static_cast<uint8_t>(MovementInputType::UP))
      deltaY -= 1.0f;
    if (val & static_cast<uint8_t>(MovementInputType::DOWN))
      deltaY += 1.0f;
    if (val & static_cast<uint8_t>(MovementInputType::LEFT))
      deltaX -= 1.0f;
    if (val & static_cast<uint8_t>(MovementInputType::RIGHT))
      deltaX += 1.0f;
  }

  float length = std::sqrt(deltaX * deltaX + deltaY * deltaY);
  if (length > 0.0f) {
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
