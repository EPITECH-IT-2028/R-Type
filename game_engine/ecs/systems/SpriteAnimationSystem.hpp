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
          
      void setSelectedRow(Entity entity, int row) {
        auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
        auto& sprite = _ecsManager.getComponent<SpriteComponent>(entity);
        
        animation.setSelectedRow(row);
        sprite.sourceRect = animation.getCurrentFrameRect();        
      }
      
      void initializeAnimation(Entity entity, Texture2D texture) {
        auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
        auto& sprite = _ecsManager.getComponent<SpriteComponent>(entity);
        
        animation.initializeFromTexture(texture.width, texture.height);
        sprite.sourceRect = animation.getCurrentFrameRect();
      }

    private:
      ECSManager &_ecsManager;
  };
} // namespace ecs