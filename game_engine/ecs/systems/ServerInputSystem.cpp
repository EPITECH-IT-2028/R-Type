#include "ServerInputSystem.hpp"
#include "Events.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "SpeedComponent.hpp"

ecs::ServerInputSystem::ServerInputSystem() {
}

void ecs::ServerInputSystem::update(float deltaTime) {
  if (_pendingInputs.empty() || !_eventQueue)
    return;
  for (auto &[entityId, inputs] : _pendingInputs) {
    if (inputs.empty())
      continue;

    for (const auto &input : inputs) {
      processInput(entityId, input, deltaTime);
      sendPositionUpdate(entityId);
    }
  }
  _pendingInputs.clear();
}

void ecs::ServerInputSystem::queueInput(Entity entityId,
                                        const PlayerInput &input) {
  for (const auto &pendingInput : _pendingInputs[entityId]) {
    if (input.input == pendingInput.input) {
      return;
    }
  }
  _pendingInputs[entityId].push_back(input);
}

void ecs::ServerInputSystem::processInput(Entity entityId,
                                          const PlayerInput &input,
                                          float deltaTime) {
  if (!_ecsManagerPtr->hasComponent<PositionComponent>(entityId) ||
      !_ecsManagerPtr->hasComponent<SpeedComponent>(entityId))
    return;

  auto &position = _ecsManagerPtr->getComponent<PositionComponent>(entityId);
  const auto &speed = _ecsManagerPtr->getComponent<SpeedComponent>(entityId);

  float moveDistance = speed.speed * deltaTime;

  switch (input.input) {
    case MovementInputType::UP:
      position.y -= moveDistance;
      break;
    case MovementInputType::DOWN:
      position.y += moveDistance;
      break;
    case MovementInputType::LEFT:
      position.x -= moveDistance;
      break;
    case MovementInputType::RIGHT:
      position.x += moveDistance;
      break;
  }

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
  _eventQueue->addRequest(positionEvent);
}
