#include "Client.hpp"
#include "BackgroundTagComponent.hpp"
#include "PositionComponent.hpp"
#include "RenderComponent.hpp"
#include "RenderManager.hpp"
#include "VelocityComponent.hpp"
#include "systems/BackgroundSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/RenderSystem.hpp"

namespace client {
  Client::Client() : _ecsManager(ecs::ECSManager::getInstance()) {
    initECS();
  }

  void Client::initECS() {
    _ecsManager.registerComponent<ecs::PositionComponent>();
    _ecsManager.registerComponent<ecs::VelocityComponent>();
    _ecsManager.registerComponent<ecs::RenderComponent>();
    _ecsManager.registerComponent<ecs::BackgroundTagComponent>();

    _ecsManager.registerSystem<ecs::BackgroundSystem>();
    _ecsManager.registerSystem<ecs::MovementSystem>();
    _ecsManager.registerSystem<ecs::RenderSystem>();

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

    Image backgroundImage = LoadImage(BG_PATH);
    float screenHeight = GetScreenHeight();
    float aspectRatio = 1.0f;
    float scaledWidth = screenHeight;
    if (backgroundImage.data != nullptr && backgroundImage.height > 0) {
      aspectRatio =
          static_cast<float>(backgroundImage.width) / static_cast<float>(backgroundImage.height);
      scaledWidth = screenHeight * aspectRatio;
      UnloadImage(backgroundImage);
    }

    auto background1 = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(background1, {0.0f, 0.0f});
    _ecsManager.addComponent<ecs::VelocityComponent>(background1, {-renderManager::SCROLL_SPEED, 0.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(
        background1, {BG_PATH});
    _ecsManager.addComponent<ecs::BackgroundTagComponent>(background1, {});

    auto background2 = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(background2, {scaledWidth, 0.0f});
    _ecsManager.addComponent<ecs::VelocityComponent>(background2, {-renderManager::SCROLL_SPEED, 0.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(
        background2, {BG_PATH});
    _ecsManager.addComponent<ecs::BackgroundTagComponent>(background2, {});
  }
}  // namespace client
