#include "MetricsSystem.hpp"
#include "ECSManager.hpp"
#include "LocalPlayerTagComponent.hpp"
#include "Macro.hpp"
#include "PacketLossComponent.hpp"
#include "PingComponent.hpp"
#include "raylib.h"

namespace ecs {
  ecs::MetricsSystem::~MetricsSystem() noexcept {
  }

  void ecs::MetricsSystem::update(float deltaTime) {
    (void)deltaTime;

    for (Entity entity : _entities) {
      if (_ecsManager.hasComponent<ecs::LocalPlayerTagComponent>(entity)) {
        if (_ecsManager.hasComponent<PingComponent>(entity)) {
          auto &pingComp = _ecsManager.getComponent<ecs::PingComponent>(entity);
          DrawText(TextFormat("Ping: %u ms", pingComp.ping), WINDOW_WIDTH - 110, 10, 20, GREEN);
        }

        if (_ecsManager.hasComponent<PacketLossComponent>(entity)) {
          auto &packetLossComp =
              _ecsManager.getComponent<ecs::PacketLossComponent>(entity);
          DrawText(TextFormat("Packet Loss: %.2f %%", packetLossComp.packetLoss * 100.0),
                   WINDOW_WIDTH - 218, 40, 20, RED);
        }
      }
    }
  }
}  // namespace ecs