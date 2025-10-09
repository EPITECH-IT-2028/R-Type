#include "SpriteAnimationSystem.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include "raylib.h"

void ecs::SpriteAnimationSystem::update(float deltaTime) {
  for (Entity entity : _entities) {
    auto &sprite = _ecsManager.getComponent<SpriteComponent>(entity);
    auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);

    if (animation.isPlaying) {
      animation.frameTimer += deltaTime;
      if (animation.frameTime <= 0.0f)
        animation.currentFrame++;
      else {
        while (animation.frameTimer >= animation.frameTime) {
          animation.frameTimer -= animation.frameTime;
          animation.currentFrame++;
        }
      }

      if (animation.currentFrame > animation.endFrame) {
        if (animation.loop)
          animation.currentFrame = animation.startFrame;
        else {
          animation.currentFrame = animation.endFrame;
          animation.isPlaying = false;
        }
      }
    }

    sprite.sourceRect = getCurrentFrameRect(entity);
  }
}

void ecs::SpriteAnimationSystem::setSelectedRow(Entity entity, int row) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);

  if (row >= 0 && row < animation.totalRows) {
    animation.selectedRow = row;
    animation.selectedColumn = -1;
    animation.currentFrame = animation.startFrame;
    animation.frameTimer = 0.0f;
  }
}

void ecs::SpriteAnimationSystem::setSelectedColumn(Entity entity, int column) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);

  if (column >= 0 && column < animation.totalColumns) {
    animation.selectedColumn = column;
    animation.selectedRow = -1;
    animation.currentFrame = animation.startFrame;
    animation.frameTimer = 0.0f;
  }
}

void ecs::SpriteAnimationSystem::setAnimationRange(Entity entity, int start,
                                                   int end) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);

  int totalFrames = 0;
  if (animation.selectedRow != -1)
    totalFrames = animation.totalColumns;
  else if (animation.selectedColumn != -1)
    totalFrames = animation.totalRows;
  else
    totalFrames = animation.totalColumns * animation.totalRows;

  if (start < 0 || end < start || end >= totalFrames)
    return;
  animation.startFrame = start;
  animation.endFrame = end;
  animation.currentFrame = start;
  animation.frameTimer = 0.0f;
}

void ecs::SpriteAnimationSystem::play(Entity entity) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.isPlaying = true;
}

void ecs::SpriteAnimationSystem::pause(Entity entity) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.isPlaying = false;
}

void ecs::SpriteAnimationSystem::stop(Entity entity) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.isPlaying = false;
  animation.currentFrame = animation.startFrame;
  animation.frameTimer = 0.0f;
}

void ecs::SpriteAnimationSystem::restart(Entity entity) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.currentFrame = animation.startFrame;
  animation.frameTimer = 0.0f;
  animation.isPlaying = true;
}

Rectangle ecs::SpriteAnimationSystem::getCurrentFrameRect(Entity entity) const {
  const auto &animation =
      _ecsManager.getComponent<SpriteAnimationComponent>(entity);

  int totalFrames;
  if (animation.selectedRow != -1)
    totalFrames = animation.totalColumns;
  else if (animation.selectedColumn != -1)
    totalFrames = animation.totalRows;
  else
    totalFrames = animation.totalColumns * animation.totalRows;

  int start = std::max(0, std::min(animation.startFrame, totalFrames - 1));
  int end = std::max(start, std::min(animation.endFrame, totalFrames - 1));
  int safeFrame = std::max(start, std::min(animation.currentFrame, end));

  int column = 0;
  int row = 0;

  if (animation.selectedRow != -1) {
    row = animation.selectedRow;
    column = safeFrame;
  } else if (animation.selectedColumn != -1) {
    column = animation.selectedColumn;
    row = safeFrame;
  } else {
    column = safeFrame % animation.totalColumns;
    row = safeFrame / animation.totalColumns;
  }

  return {static_cast<float>(column * animation.frameWidth),
          static_cast<float>(row * animation.frameHeight),
          static_cast<float>(animation.frameWidth),
          static_cast<float>(animation.frameHeight)};
}

void ecs::SpriteAnimationSystem::initializeFromTexture(Entity entity,
                                                       int textureWidth,
                                                       int textureHeight) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);

  if (animation.totalColumns <= 0 || animation.totalRows <= 0) {
    std::cerr << "Error: Invalid animation dimensions for entity " << entity
              << " (columns=" << animation.totalColumns
              << ", rows=" << animation.totalRows << ")\n";
    return;
  }

  animation.frameWidth = textureWidth / animation.totalColumns;
  animation.frameHeight = textureHeight / animation.totalRows;
}
