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

constexpr int CHALLENGE_HEX_LEN = 129;
constexpr std::uint32_t INVALID_ID = std::numeric_limits<std::uint32_t>::max();
constexpr std::uint32_t INVALID_ENTITY =
    std::numeric_limits<std::uint32_t>::max();

constexpr std::uint32_t SERVER_SENDER_ID =
    std::numeric_limits<std::uint32_t>::max();

/* UI constants */
namespace chatUI {
  constexpr int BOX_MAX_TEXT_LEN = 170;
  constexpr int BOX_BOTTOM_OFFSET = 80;

  constexpr int LINE_HEIGHT = 25;
  constexpr int FONT_SIZE = 20;

  constexpr int INPUT_LEFT_OFFSET = 10;
  constexpr int INPUT_BOTTOM_OFFSET = 40;
  constexpr int INPUT_RIGHT_MARGIN = 20;
  constexpr int INPUT_HEIGHT = 30;
  constexpr float INPUT_ROUNDNESS = 0.5f;
  constexpr int INPUT_TEXT_OFFSET = 15;
  constexpr int INPUT_TEXT_Y_OFFSET = 35;
  constexpr int INPUT_TEXT_RIGHT_MARGIN = 50;
}  // namespace chatUI

/* ASCII characters */
constexpr unsigned char ASCII_NULL = 0x00;
constexpr unsigned char ASCII_SPACE = 0x20;
constexpr unsigned char ASCII_DEL = 0x7f;

constexpr const char *SQL_PATH = "db.sql";
constexpr int TPS = 60;
constexpr int NANOSECONDS_IN_SECOND = 1000000000;

/* Serialization */
constexpr std::size_t SERIALIZE_32_BYTES = 32;
constexpr std::size_t SERIALIZE_512_BYTES = 512;
