#pragma once
#include "raylib.h"
#include <vector>

namespace ecs {
  struct SpriteAnimationComponent {
    // Configuration de la sprite sheet
    int totalRows;
    int totalColumns;
    int selectedRow;
        
    // Animation
    int currentFrame;
    int startFrame;
    int endFrame;
    float frameTime;
    float frameTimer;
    bool isPlaying;
    bool loop;
    
    // Dimensions calculées
    int frameWidth;
    int frameHeight;
        
    SpriteAnimationComponent() 
      : totalRows(1), 
        totalColumns(1), 
        selectedRow(0),
        currentFrame(0),
        startFrame(0),
        endFrame(0),
        frameTime(0.15f), 
        frameTimer(0.0f), 
        isPlaying(true), 
        loop(true),
        frameWidth(32),
        frameHeight(32) {}
              
    SpriteAnimationComponent(int rows, int cols, float fTime = 0.15f)
      : totalRows(rows),
        totalColumns(cols),
        selectedRow(0),
        currentFrame(0),
        startFrame(0),
        endFrame(cols - 1),
        frameTime(fTime),
        frameTimer(0.0f),
        isPlaying(true),
        loop(true),
        frameWidth(32),
        frameHeight(32) {}
              
    void initializeFromTexture(int textureWidth, int textureHeight) {
      frameWidth = textureWidth / totalColumns;
      frameHeight = textureHeight / totalRows;
    }
        
    Rectangle getCurrentFrameRect() const {
      return {
        static_cast<float>(currentFrame * frameWidth),
        static_cast<float>(selectedRow * frameHeight),
        static_cast<float>(frameWidth),
        static_cast<float>(frameHeight)
      };
    }
        
    void setSelectedRow(int row) {
      if (row >= 0 && row < totalRows) {
        selectedRow = row;
        currentFrame = startFrame;
        frameTimer = 0.0f;
      }
    }
        
    void setAnimationRange(int start, int end) {
      startFrame = start;
      endFrame = end;
      currentFrame = startFrame;
      frameTimer = 0.0f;
    }
        
    void play() { isPlaying = true; }
    void pause() { isPlaying = false; }
    void stop() { isPlaying = false; currentFrame = startFrame; frameTimer = 0.0f; }
    void restart() { currentFrame = startFrame; frameTimer = 0.0f; isPlaying = true; }
  };
} // namespace ecs