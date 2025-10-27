#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
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

constexpr int CLIENT_TIMEOUT = 45;

constexpr int HEARTBEAT_INTERVAL_CLIENT = 15;

constexpr float TOLERANCE = 2.0f;

constexpr float FPS = 60.0f;

constexpr float PLAYER_SPEED = 250.0f;

constexpr float ENEMY_SPEED = -80.0f;

constexpr int ENEMY_SPAWN_Y = 700;

constexpr float ENEMY_SPAWN_X = 1220.0f;

constexpr int ENEMY_SPAWN_OFFSET = 25;

constexpr float PROJECTILE_SPEED = 100.0f;

constexpr int WINDOW_HEIGHT = 750;
constexpr int WINDOW_WIDTH = 1200;
constexpr float PLAYER_WIDTH = 66.0f;
constexpr float PLAYER_HEIGHT = 37.0f;

constexpr int MARGIN_WINDOW = 30;

constexpr std::uint32_t NO_ROOM = std::numeric_limits<std::uint32_t>::max();

constexpr int COUNTDOWN_TIME = 5;

constexpr int MAX_ROOMS = 10;

constexpr int INVALID_ID = -1;

/* ASCII characters */
constexpr char ASCII_NULL = 0;
constexpr char ASCII_SPACE = 32;
constexpr char ASCII_DEL = 127;
