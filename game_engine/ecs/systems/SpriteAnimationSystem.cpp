#include "SpriteAnimationSystem.hpp"
#include <algorithm>
#include <iostream>
#include "raylib.h"

/**
 * @brief Advance sprite animations for all tracked entities and update their
 * source rectangles.
 *
 * Advances each entity's SpriteAnimationComponent based on the elapsed time and
 * the component's configuration: when the animation is playing and `frameTime`
 * is non-zero, accumulated time is used to move the current frame forward
 * (positive `frameTime`) or backward (negative `frameTime`). When the animation
 * passes its start/end bounds it will either loop back to the configured
 * start/end frame if `loop` is true, or clamp to the final frame if `loop` is
 * false. After advancing frames, the sprite's `sourceRect` is refreshed from
 * the current frame.
 *
 * @param deltaTime Elapsed time in seconds since the last update.
 */
void ecs::SpriteAnimationSystem::update(float deltaTime) {
  for (Entity entity : _entities) {
    auto &sprite = _ecsManager.getComponent<SpriteComponent>(entity);
    auto &animation =
        _ecsManager.getComponent<SpriteAnimationComponent>(entity);

    if (animation.isPlaying && animation.frameTime != 0) {
      animation.frameTimer += deltaTime;

      while (animation.frameTimer >= std::abs(animation.frameTime)) {
        animation.frameTimer -= std::abs(animation.frameTime);
        if (animation.frameTime > 0) {
          animation.currentFrame++;
        } else {
          animation.currentFrame--;
        }
      }

      bool finished = (animation.frameTime > 0 &&
                       animation.currentFrame > animation.endFrame) ||
                      (animation.frameTime < 0 &&
                       animation.currentFrame < animation.startFrame);

      if (finished) {
        if (animation.loop) {
          animation.currentFrame = (animation.frameTime > 0)
                                       ? animation.startFrame
                                       : animation.endFrame;
        } else {
          animation.currentFrame = (animation.frameTime > 0)
                                       ? animation.endFrame
                                       : animation.startFrame;
        }
      }
    }

    sprite.sourceRect = getCurrentFrameRect(entity);
  }
}
/**
 * @brief Selects a specific sprite sheet row for the entity's animation.
 *
 * If the provided row is within [0, totalRows-1], sets that row as the active
 * selection, clears any column selection, resets the current frame to the
 * animation's start frame, and resets the frame timer to 0. Does nothing when
 * the row is out of range.
 *
 * @param entity The entity whose animation will be modified.
 * @param row Zero-based index of the row to select.
 */
void ecs::SpriteAnimationSystem::setSelectedRow(Entity entity, int row) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);

  if (row >= 0 && row < animation.totalRows) {
    animation.selectedRow = row;
    animation.selectedColumn = -1;
    animation.currentFrame = animation.startFrame;
    animation.frameTimer = 0.0f;
  }
}

/**
 * @brief Selects a specific column of frames for the entity's animation and
 * resets playback state.
 *
 * If `column` is within the entity animation's valid range, sets the animation
 * to use that column, clears any row selection, resets the current frame to the
 * animation's start frame, and resets the frame timer.
 *
 * @param entity The entity whose animation will be modified.
 * @param column Column index to select (must be between 0 and `totalColumns -
 * 1`).
 */
void ecs::SpriteAnimationSystem::setSelectedColumn(Entity entity, int column) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);

  if (column >= 0 && column < animation.totalColumns) {
    animation.selectedColumn = column;
    animation.selectedRow = -1;
    animation.currentFrame = animation.startFrame;
    animation.frameTimer = 0.0f;
  }
}

/**
 * @brief Sets the active frame range for an entity's sprite animation and
 * resets playback to the range start.
 *
 * Updates the animation's startFrame and endFrame to the provided values, sets
 * currentFrame to start, and resets the internal frame timer. If the requested
 * range is invalid (start < 0, end < start, or end >= number of available
 * frames for the current selection mode), the function makes no changes.
 *
 * @param entity Entity whose animation range will be changed.
 * @param start Index of the first frame in the new range (inclusive).
 * @param end Index of the last frame in the new range (inclusive).
 */
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

/**
 * @brief Start playback of the sprite animation for the specified entity.
 *
 * Marks the entity's SpriteAnimationComponent as playing so frame advancement
 * will occur during subsequent updates.
 *
 * @param entity Entity whose sprite animation should be started (must have a
 * SpriteAnimationComponent).
 */
void ecs::SpriteAnimationSystem::play(Entity entity) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.isPlaying = true;
}

/**
 * @brief Pauses playback of the sprite animation for the specified entity.
 *
 * Stops the entity's sprite animation from advancing frames until playback is
 * resumed.
 *
 * @param entity The entity whose sprite animation will be paused.
 */
void ecs::SpriteAnimationSystem::pause(Entity entity) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.isPlaying = false;
}

/**
 * @brief Stops the sprite animation for the specified entity and resets it to
 * the configured start frame.
 *
 * Sets the animation's playback state to stopped, resets the current frame to
 * the animation's startFrame, and clears the internal frame timer.
 *
 * @param entity The entity whose sprite animation will be stopped and reset.
 */
void ecs::SpriteAnimationSystem::stop(Entity entity) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.isPlaying = false;
  animation.currentFrame = animation.startFrame;
  animation.frameTimer = 0.0f;
}

/**
 * @brief Restart the animation for the given entity and begin playback from its
 * start frame.
 *
 * Resets the animation's current frame to its configured start frame, clears
 * the frame timer, and marks the animation as playing.
 *
 * @param entity Entity whose SpriteAnimationComponent will be restarted.
 */
void ecs::SpriteAnimationSystem::restart(Entity entity) {
  auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);
  animation.currentFrame = animation.startFrame;
  animation.frameTimer = 0.0f;
  animation.isPlaying = true;
}

/**
 * @brief Computes the source rectangle for the entity's current animation
 * frame.
 *
 * Determines the valid frame index based on the component's selected row/column
 * or full-grid mode, constrains start/end/current frames into the valid range,
 * and maps the chosen frame to its column and row in the sprite sheet.
 *
 * @param entity Entity whose current animation frame rectangle is requested.
 * @return Rectangle Source rectangle (x, y, width, height) for the current
 * frame in texture coordinates.
 */
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

/**
 * @brief Initializes the animation frame dimensions from a texture's size.
 *
 * Computes and stores each frame's width and height using the entity's
 * configured totalColumns and totalRows. If the animation's column or row
 * count is not greater than zero, the function logs an error and leaves the
 * component unchanged.
 *
 * @param entity Entity whose SpriteAnimationComponent will be initialized.
 * @param textureWidth Width of the source texture in pixels.
 * @param textureHeight Height of the source texture in pixels.
 */
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
