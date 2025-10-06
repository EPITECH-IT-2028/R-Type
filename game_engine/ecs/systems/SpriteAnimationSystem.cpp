#include "SpriteAnimationSystem.hpp"

void ecs::SpriteAnimationSystem::update(float deltaTime) {
  for (Entity entity : _entities) {
    auto& sprite = _ecsManager.getComponent<SpriteComponent>(entity);
    auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
    
    if (!animation.isPlaying) {
        continue;
    } 
    animation.frameTimer += deltaTime;
    if (animation.frameTimer >= animation.frameTime) {
      animation.currentFrame++;
      animation.frameTimer = 0.0f;
        
      if (animation.currentFrame > animation.endFrame) {
        if (animation.loop) {
          animation.currentFrame = animation.startFrame;
        } else {
          animation.currentFrame = animation.endFrame;
          animation.isPlaying = false;
        }
      }
      sprite.sourceRect = getCurrentFrameRect(entity);
    }
  }
}

void ecs::SpriteAnimationSystem::setSelectedRow(Entity entity, int row) {
  auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  auto& sprite = _ecsManager.getComponent<SpriteComponent>(entity);
  
  if (row >= 0 && row < animation.totalRows) {
    animation.selectedRow = row;
    animation.currentFrame = animation.startFrame;
    animation.frameTimer = 0.0f;
  }
  sprite.sourceRect = getCurrentFrameRect(entity);
}

void ecs::SpriteAnimationSystem::setAnimationRange(Entity entity, int start, int end) {
  auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  
  animation.startFrame = start;
  animation.endFrame = end;
  animation.currentFrame = start;
  animation.frameTimer = 0.0f;
}

void ecs::SpriteAnimationSystem::play(Entity entity) {
  auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.isPlaying = true;
}

void ecs::SpriteAnimationSystem::pause(Entity entity) {
  auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.isPlaying = false;
}

void ecs::SpriteAnimationSystem::stop(Entity entity) {
  auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.isPlaying = false;
  animation.currentFrame = animation.startFrame;
  animation.frameTimer = 0.0f;
}

void ecs::SpriteAnimationSystem::restart(Entity entity) {
  auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.currentFrame = animation.startFrame;
  animation.frameTimer = 0.0f;
  animation.isPlaying = true;
}

Rectangle ecs::SpriteAnimationSystem::getCurrentFrameRect(Entity entity) const {
  const auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  
  return {
    static_cast<float>(animation.currentFrame * animation.frameWidth),
    static_cast<float>(animation.selectedRow * animation.frameHeight),
    static_cast<float>(animation.frameWidth),
    static_cast<float>(animation.frameHeight)
  };
}

void ecs::SpriteAnimationSystem::initializeFromTexture(Entity entity, int textureWidth, int textureHeight) {
  auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  
  animation.frameWidth = textureWidth / animation.totalColumns;
  animation.frameHeight = textureHeight / animation.totalRows;
}

void ecs::SpriteAnimationSystem::initializeAnimation(Entity entity, Texture2D texture) {
  auto& animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  auto& sprite = _ecsManager.getComponent<SpriteComponent>(entity);
  
  initializeFromTexture(entity, texture.width, texture.height);
  sprite.sourceRect = getCurrentFrameRect(entity);
}