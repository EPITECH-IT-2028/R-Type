#pragma once
#include "raylib.h"

namespace ecs {
  struct SpriteAnimationComponent {
      int totalRows = 1;
      int totalColumns = 1;
      int selectedRow = -1;
      int selectedColumn = -1;

      int currentFrame = 0;
      int startFrame = 0;
      int endFrame = 0;
      float frameTime = 0.15f;
      float frameTimer = 0.0f;
      bool isPlaying = true;
      bool loop = true;
      int neutralFrame = 0;

      int frameWidth = 0;
      int frameHeight = 0;
      bool isInitialized = false;
  };
}  // namespace ecs