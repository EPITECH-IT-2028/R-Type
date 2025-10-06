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
      void setAnimationRange(Entity entity, int start, int end);
      void play(Entity entity);
      void pause(Entity entity);
      void stop(Entity entity);
      void restart(Entity entity);   
   
      void initializeAnimation(Entity entity, Texture2D texture);
      void initializeFromTexture(Entity entity, int textureWidth, int textureHeight);
      Rectangle getCurrentFrameRect(Entity entity) const;

    private:
      ECSManager &_ecsManager;
  };
} // namespace ecs