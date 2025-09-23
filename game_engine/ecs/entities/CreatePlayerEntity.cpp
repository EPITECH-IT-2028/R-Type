#include "ECSManager.hpp"
#include "PositionComponent.hpp"
#include "HealthComponent.hpp"
#include "PlayerComponent.hpp"
#include "SpeedComponent.hpp"
#include "VelocityComponent.hpp"

void CreatePlayerEntity(ecs::ECSManager &ecsManager) {
    Entity player = ecsManager.createEntity();

    // Add components to the player entity
    // ecsManager.addComponent(player, ecs::PlayerComponent{"Player1", true, 0, true});
    ecsManager.addComponent(player, ecs::HealthComponent{100});
    ecsManager.addComponent(player, ecs::PositionComponent{0.0f, 0.0f});
    ecsManager.addComponent(player, ecs::SpeedComponent{0.0f});
    ecsManager.addComponent(player, ecs::VelocityComponent{0.0f, 0.0f});

    // Add systems to the player entity
}