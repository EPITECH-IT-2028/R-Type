#pragma once

#define PLATFORM_DESKTOP

#include "raylib.h"

#if defined(_WIN32)
  #undef LoadImage
#endif
