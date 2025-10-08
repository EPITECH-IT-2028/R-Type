#pragma once

#include <cstddef>
#ifdef ERROR
  #undef ERROR
#endif

constexpr std::size_t BUFFER_SIZE = 2048;
/* Macros for function's returns */
constexpr int OK = 0;
constexpr int KO = -1;

/* Macros for ports */
constexpr int MAX_PORT = 65535;
constexpr int MIN_PORT = 1;

/* Macros for game */
constexpr int COLLISION_DAMAGE = 20;

/* Macros for file paths */
constexpr const char *SERVER_PROPERTIES = "server/server.properties";

constexpr float CONVERT_MS_TO_S = 1000.0f;

constexpr float TOLERANCE = 2.0f;

constexpr float FPS = 60.0f;
