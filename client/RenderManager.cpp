#include "RenderManager.hpp"
#include <raylib.h>
#include <cstdio>
#include <cstdlib>
#include "Macro.hpp"
#include "RaylibUtils.hpp"

namespace renderManager {
  /**
   * @brief Configure graphics, create the application window, and initialize renderer state.
   *
   * Initializes windowing flags and attempts to create a window with the provided dimensions and title.
   * Records whether initialization succeeded, sets a 60 FPS target if vertical sync is not active,
   * registers the renderer's colored log callback, and applies the configured log level.
   *
   * @param width Initial window width in pixels.
   * @param height Initial window height in pixels.
   * @param title Window title string.
   */
  Renderer::Renderer(int width, int height, const char *title) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(width, height, title);
    _initSucceeded = IsWindowReady();
    if (!IsWindowState(FLAG_VSYNC_HINT))
      SetTargetFPS(60);
    SetTraceLogCallback(coloredLog);
    utils::setLogLevel();
  }

  /**
   * @brief Closes the renderer's window and releases associated graphics resources.
   *
   * Ensures the underlying window created by the renderer is closed before teardown.
   */
  Renderer::~Renderer() {
    CloseWindow();
  }

  bool Renderer::shouldClose() const {
    return WindowShouldClose();
  }

  void Renderer::beginDrawing() const {
    BeginDrawing();
  }

  /**
   * @brief Clears the current drawing surface using the specified color.
   *
   * @param color Color used to fill the background for the current frame.
   */
  void Renderer::clearBackground(Color color) const {
    ClearBackground(color);
  }

  /**
   * @brief Ends the current frame's drawing phase and presents the rendered frame to the display.
   *
   * Finalizes all drawing for the current frame and submits the result to be shown on screen.
   */
  void Renderer::endDrawing() const {
    EndDrawing();
  }

  /**
   * @brief Adjusts the application window to preserve the target aspect ratio.
   *
   * Computes a new window width and height that maintain the aspect ratio defined
   * by WINDOW_WIDTH and WINDOW_HEIGHT based on the current screen size, then
   * applies the computed dimensions to the window.
   *
   * @note Chooses whether to preserve width or height by comparing the absolute
   * difference between the current screen dimensions and the target dimensions,
   * favoring the dimension with the larger delta.
   */
  void Renderer::resizeWindow() {
    int newWidth = 0;
    int newHeight = 0;
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    const int deltaWidth = abs(width - WINDOW_WIDTH);
    const int deltaHeight = abs(height - WINDOW_HEIGHT);
    const float aspectRatio =
        static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

    if (deltaWidth > deltaHeight) {
      newWidth = width;
      newHeight = static_cast<int>(newWidth / aspectRatio);
    } else {
      newHeight = height;
      newWidth = static_cast<int>(newHeight * aspectRatio);
    }
    SetWindowSize(newWidth, newHeight);
  }

  /**
   * @brief Draws a string of text at the specified screen coordinates using the given font size and color.
   *
   * @param text Null-terminated UTF-8 string to draw.
   * @param posX X coordinate in pixels for the text baseline start.
   * @param posY Y coordinate in pixels for the text baseline start.
   * @param fontSize Font size in pixels.
   * @param color Color used to render the text.
   */
  void Renderer::drawText(const char *text, int posX, int posY, int fontSize,
                          Color color) {
    DrawText(text, posX, posY, fontSize, color);
  }

  /**
   * @brief Draws a filled rectangle at the specified position.
   *
   * @param posX X coordinate of the rectangle's top-left corner in pixels.
   * @param posY Y coordinate of the rectangle's top-left corner in pixels.
   * @param width Width of the rectangle in pixels.
   * @param height Height of the rectangle in pixels.
   * @param color Fill color of the rectangle.
   */
  void Renderer::drawRectangle(int posX, int posY, int width, int height,
                               Color color) {
    DrawRectangle(posX, posY, width, height, color);
  }

  /**
   * @brief Draws a filled rectangle with rounded corners.
   *
   * Draws a filled rectangle at the given pixel position and size using the specified corner roundness and color.
   *
   * @param posX X coordinate of the rectangle's top-left corner in pixels.
   * @param posY Y coordinate of the rectangle's top-left corner in pixels.
   * @param width Width of the rectangle in pixels.
   * @param height Height of the rectangle in pixels.
   * @param roundness Corner roundness as a value between 0.0 (square corners) and 1.0 (maximum rounding).
   * @param color Fill color for the rectangle.
   */
  void Renderer::drawRectangleRounded(int posX, int posY, int width, int height,
                                      float roundness, Color color) {
    Rectangle rec = {static_cast<float>(posX), static_cast<float>(posY),
                     static_cast<float>(width), static_cast<float>(height)};
    DrawRectangleRounded(rec, roundness, 16, color);
  }

  /**
   * @brief Logs a formatted message to stdout with a colored level prefix.
   *
   * Prints a level tag (colorized for INFO, ERROR, WARNING, DEBUG) followed by the message formatted
   * using the provided printf-style format string and arguments, then a newline.
   *
   * @param msgType Message level selector; expected values include LOG_INFO, LOG_ERROR, LOG_WARNING, LOG_DEBUG.
   * @param text printf-style format string for the message body.
   * @param args va_list of arguments matching `text`.
   */
  void Renderer::coloredLog(int msgType, const char *text, va_list args) {
    switch (msgType) {
      case LOG_INFO:
        printf("[\e[1;32mINFO\e[0m] : ");
        break;
      case LOG_ERROR:
        printf("[\e[1;31mERROR\e[0m]: ");
        break;
      case LOG_WARNING:
        printf("[\e[1;33mWARN\e[0m] : ");
        break;
      case LOG_DEBUG:
        printf("[\e[1;34mDEBUG\e[0m]: ");
        break;
      default:
        break;
    }

    vprintf(text, args);
    printf("\n");
  }

}  // namespace renderManager