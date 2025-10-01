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
      sprite.sourceRect = animation.getCurrentFrameRect();
    }
  }
}