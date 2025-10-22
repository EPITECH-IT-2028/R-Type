#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include "raylib.h"

namespace asset {
  /**
   * @brief Manages embedded assets and provides runtime loading capabilities.
   *
   * This class encapsulates asset embedding logic, allowing assets to be
   * exported as code at compile-time and loaded at runtime without requiring
   * external files.
   */
  class AssetManager {
    public:
      /**
       * @brief Exports an image file as a C header file with embedded data.
       *
       * Uses raylib's ExportImageAsCode() to generate a header file containing
       * the image data as a static array. The generated file can be included
       * at compile-time to embed assets directly in the executable.
       *
       * @param imagePath Path to the source image file
       * @param outputHeaderPath Path where the generated header will be saved
       * @return true if export succeeded, false otherwise
       */
      static bool exportImageAsCode(const std::string &imagePath,
                                    const std::string &outputHeaderPath);

      /**
       * @brief Loads a texture from embedded image data or falls back to file.
       *
       * Attempts to load from embedded data first using the "embedded://"
       * scheme. If the asset is not embedded, falls back to loading from the
       * file system.
       *
       * @param path Asset path (use "embedded://name" for embedded assets)
       * @return Loaded texture (check texture.id != 0 for success)
       */
      static Texture2D loadTexture(const std::string &path);

      /**
       * @brief Loads an Image from embedded data or falls back to file.
       *
       * Similar to loadTexture, but returns an Image instead of a Texture2D.
       * Note: DO NOT call UnloadImage() on embedded images, as they use static
       * data.
       *
       * @param path Asset path (use "embedded://name" for embedded assets)
       * @return Loaded image (check image.data != nullptr for success)
       */
      static Image loadImage(const std::string &path);

      /**
       * @brief Registers an embedded image for runtime loading.
       *
       * Associates an embedded image name with its data pointer, allowing
       * the AssetManager to load it at runtime via loadTexture().
       *
       * @param name Name identifier for the embedded asset
       * @param data Pointer to the embedded image data
       * @param width Width of the embedded image
       * @param height Height of the embedded image
       * @param format Pixel format of the embedded image
       */
      static void registerEmbeddedImage(const std::string &name, void *data,
                                        int width, int height, int format);

    private:
      struct EmbeddedImageData {
          void *data;
          int width;
          int height;
          int format;
      };

      static std::unordered_map<std::string, EmbeddedImageData> _embeddedImages;
      static std::mutex _mutex;
  };
}  // namespace asset
