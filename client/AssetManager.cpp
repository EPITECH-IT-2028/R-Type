#include "AssetManager.hpp"
#include <raylib.h>
#include <iostream>

namespace asset {
  std::unordered_map<std::string, AssetManager::EmbeddedImageData>
      AssetManager::_embeddedImages;
  std::mutex AssetManager::_mutex;

  bool AssetManager::exportImageAsCode(const std::string &imagePath,
                                       const std::string &outputHeaderPath) {
    Image image = LoadImage(imagePath.c_str());

    if (image.data == nullptr) {
      std::cerr << "[ERROR] Failed to load image: " << imagePath << std::endl;
      return false;
    }
    bool success = ExportImageAsCode(image, outputHeaderPath.c_str());
    UnloadImage(image);
    if (success)
      std::cout << "[INFO] Exported " << imagePath << " to " << outputHeaderPath
                << std::endl;
    else
      std::cerr << "[ERROR] Failed to export image to: " << outputHeaderPath
                << " (check file permissions and disk space)" << std::endl;
    return success;
  }

  Texture2D AssetManager::loadTexture(const std::string &path) {
    const std::string prefix = "embedded://";

    if (path.find(prefix) == 0) {
      std::string assetName = path.substr(prefix.length());
      std::lock_guard<std::mutex> lock(_mutex);
      auto idx = _embeddedImages.find(assetName);

      if (idx != _embeddedImages.end()) {
        const EmbeddedImageData &imgData = idx->second;
        Image img = {.data = imgData.data,
                     .width = imgData.width,
                     .height = imgData.height,
                     .mipmaps = 1,
                     .format = imgData.format};
        Texture2D texture = LoadTextureFromImage(img);
        return texture;
      } else {
        TraceLog(LOG_WARNING, "[WARN] Embedded asset not found: %s",
                 assetName.c_str());
        Texture2D texture;
        return Texture2D{
          .id = 0,
          .width = 0,
          .height = 0,
          .mipmaps = 1,
          .format = 0
        };
      }
    }
    return LoadTexture(path.c_str());
  }

  Image AssetManager::loadImage(const std::string &path) {
    const std::string prefix = "embedded://";

    if (path.find(prefix) == 0) {
      std::string assetName = path.substr(prefix.length());
      std::lock_guard<std::mutex> lock(_mutex);
      auto idx = _embeddedImages.find(assetName);

      if (idx != _embeddedImages.end()) {
        const EmbeddedImageData &imgData = idx->second;
        Image img = {.data = imgData.data,
                     .width = imgData.width,
                     .height = imgData.height,
                     .mipmaps = 1,
                     .format = imgData.format};
        return img;
      } else {
        TraceLog(LOG_WARNING, "[WARN] Embedded asset not found: %s",
                 assetName.c_str());
        return Image{
          .data = nullptr,
          .width = 0,
          .height = 0,
          .mipmaps = 1,
          .format = 0
        };
      }
    }
    return LoadImage(path.c_str());
  }

  void AssetManager::registerEmbeddedImage(const std::string &name, void *data,
                                           int width, int height, int format) {
    std::lock_guard<std::mutex> lock(_mutex);
    _embeddedImages[name] = {data, width, height, format};
    TraceLog(LOG_INFO, "[INFO] Registered embedded image: %s (%dx%d)",
             name.c_str(), width, height);
  }
}  // namespace asset
