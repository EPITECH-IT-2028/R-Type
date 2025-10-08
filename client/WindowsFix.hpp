#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOGDI
#include <windows.h>
#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef LoadImage
#endif
