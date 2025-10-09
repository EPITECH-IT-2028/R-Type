#pragma once
#include "ECSManager.hpp"
#include "SpriteAnimationComponent.hpp"
#include "SpriteComponent.hpp"
#include "System.hpp"
#include "raylib.h"

namespace ecs {
  class SpriteAnimationSystem : public System {
    public:
      explicit SpriteAnimationSystem(
          ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {}

      void initializeAnimation(Entity entity, const std::shared_ptr<Texture2D> &texture);
      void initializeFromTexture(Entity entity, int textureWidth, int textureHeight);
      Rectangle getCurrentFrameRect(Entity entity) const;

      void update(float deltaTime) override;

      void setSelectedRow(Entity entity, int row);
      void setSelectedColumn(Entity entity, int column);
      void setAnimationRange(Entity entity, int start, int end);
      void play(Entity entity);
      void pause(Entity entity);
      void stop(Entity entity);
      void restart(Entity entity);

    private:
      ECSManager &_ecsManager;
  };
}  // namespace ecs