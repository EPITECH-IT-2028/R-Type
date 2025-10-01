#pragma once
#include "System.hpp"
#include "ECSManager.hpp"
#include "SpriteComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "raylib.h"
#include <iostream>

namespace ecs {
  class SpriteAnimationSystem : public System {
    public:
      explicit SpriteAnimationSystem(ECSManager &ecsManager = ECSManager::getInstance())
        : _ecsManager(ecsManager) {}

      void update(float deltaTime) override;
          
      void setSelectedRow(Entity entity, int row);
      
      void initializeAnimation(Entity entity, Texture2D texture);

    private:
      ECSManager &_ecsManager;
  };
} // namespace ecs