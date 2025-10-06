#include "Client.hpp"
#include "BackgroundTagComponent.hpp"
#include "InputSystem.hpp"
#include "PlayerTagComponent.hpp"
#include "PositionComponent.hpp"
#include "RenderComponent.hpp"
#include "RenderManager.hpp"
#include "SpeedComponent.hpp"
#include "VelocityComponent.hpp"
#include "systems/BackgroundSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/RenderSystem.hpp"

namespace client {
  Client::Client() : _ecsManager(ecs::ECSManager::getInstance()) {
    initECS();
  }

  void Client::initECS() {
    registerComponent();
    registerSystem();
    signSystem();

    createBackgroundEntities();
    createPlayerEntity();
  }

  void Client::registerComponent() {
    _ecsManager.registerComponent<ecs::PositionComponent>();
    _ecsManager.registerComponent<ecs::VelocityComponent>();
    _ecsManager.registerComponent<ecs::RenderComponent>();
    _ecsManager.registerComponent<ecs::SpeedComponent>();
    _ecsManager.registerComponent<ecs::BackgroundTagComponent>();
    _ecsManager.registerComponent<ecs::PlayerTagComponent>();
  }

  void Client::registerSystem() {
    _ecsManager.registerSystem<ecs::BackgroundSystem>();
    _ecsManager.registerSystem<ecs::MovementSystem>();
    _ecsManager.registerSystem<ecs::RenderSystem>();
    _ecsManager.registerSystem<ecs::InputSystem>();
  }

  void Client::signSystem() {
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::RenderComponent>());
      signature.set(_ecsManager.getComponentType<ecs::BackgroundTagComponent>());
      _ecsManager.setSystemSignature<ecs::BackgroundSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::VelocityComponent>());
      _ecsManager.setSystemSignature<ecs::MovementSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::RenderComponent>());
      _ecsManager.setSystemSignature<ecs::RenderSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::VelocityComponent>());
      signature.set(_ecsManager.getComponentType<ecs::SpeedComponent>());
      _ecsManager.setSystemSignature<ecs::InputSystem>(signature);
    }
  }

  void Client::createBackgroundEntities() {
    Image backgroundImage = LoadImage(renderManager::BG_PATH);
    float screenHeight = GetScreenHeight();
    float aspectRatio = 1.0f;
    float scaledWidth = screenHeight;
    if (backgroundImage.data != nullptr && backgroundImage.height > 0) {
      aspectRatio = static_cast<float>(backgroundImage.width) /
                    static_cast<float>(backgroundImage.height);
      scaledWidth = screenHeight * aspectRatio;
      UnloadImage(backgroundImage);
    }

    auto background1 = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(background1, {0.0f, 0.0f});
    _ecsManager.addComponent<ecs::VelocityComponent>(
        background1, {-renderManager::SCROLL_SPEED, 0.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(
        background1, {renderManager::BG_PATH});
    _ecsManager.addComponent<ecs::BackgroundTagComponent>(background1, {});

    auto background2 = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(background2, {scaledWidth, 0.0f});
    _ecsManager.addComponent<ecs::VelocityComponent>(
        background2, {-renderManager::SCROLL_SPEED, 0.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(
        background2, {renderManager::BG_PATH});
    _ecsManager.addComponent<ecs::BackgroundTagComponent>(background2, {});
  }

  void Client::createPlayerEntity() {
    auto player = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(player, {100.0f, 100.0f});
    _ecsManager.addComponent<ecs::VelocityComponent>(player, {0.0f, 0.0f});
    _ecsManager.addComponent<ecs::SpeedComponent>(player, {200.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(
        player, {renderManager::PLAYER_PATH});
    _ecsManager.addComponent<ecs::PlayerTagComponent>(player, {});
  }
}  // namespace client
