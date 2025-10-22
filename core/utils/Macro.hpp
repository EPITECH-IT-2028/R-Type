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

constexpr int NO_ROOM = -1;

constexpr int RESEND_PACKET_DELAY = 1000;  // number in milliseconds
