#pragma once

#include <cstddef>
#ifdef ERROR
  #undef ERROR
#endif

constexpr std::size_t BUFFER_SIZE = 2048;
/* Macros for function's returns */
constexpr int OK = 0;
constexpr int KO = -1;

/* Macros for game */
constexpr int COLLISION_DAMAGE = 20;

constexpr float CONVERT_MS_TO_S = 1000.0f;

constexpr float TOLERANCE = 2.0f;

constexpr float FPS = 60.0f;

constexpr int ENEMY_WINDOW_HEIGHT = 700;

constexpr float ENEMY_WINDOW_WIDTH = 1220.0f;

constexpr int ENEMY_SPAWN_OFFSET = 25;
