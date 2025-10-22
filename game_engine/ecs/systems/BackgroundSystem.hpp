#pragma once

#include <string>
#include <unordered_map>
#include "ECSManager.hpp"
#include "System.hpp"
#include "raylib.h"

namespace ecs {
  class BackgroundSystem : public System {
    public:
      explicit BackgroundSystem(
          ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {
      }
      ~BackgroundSystem() noexcept;

      void update(float deltaTime) override;

    private:
      ECSManager &_ecsManager;
      std::unordered_map<std::string, Texture2D> _textureCache;
  };
}  // namespace ecs
