#include <filesystem>
#include <iostream>
#include <string>
#include "AssetManager.hpp"

/**
 * @brief Standalone tool to export image assets as C header files.
 *
 * This tool runs at compile-time to generate header files containing
 * embedded image data. The generated headers are then included in the
 * client executable, eliminating the need for external asset files.
 *
 * Usage: ./embed_assets <resources_dir> <output_dir>
 */

constexpr int OK = 0;
constexpr int KO = -1;

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <resources_dir> <output_dir>"
              << std::endl;
    return KO;
  }

  std::string resourcesDir = argv[1];
  std::string outputDir = argv[2];
  bool success = true;
  SetTraceLogLevel(LOG_WARNING);
  try {
    std::filesystem::create_directories(outputDir);
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "[ERROR] Failed to create output directory: " << e.what()
              << std::endl;
    return KO;
  }

  std::cout << "=== Asset Exporter ===" << std::endl;
  std::cout << "Resources: " << resourcesDir << std::endl;
  std::cout << "Output: " << outputDir << std::endl;

  {
    std::string startScreenPath = resourcesDir + "/start_screen.png";
    std::string outputPath = outputDir + "/start_screen_data.h";

    if (!asset::AssetManager::exportImageAsCode(startScreenPath, outputPath)) {
      std::cerr << "[ERROR] Failed to export start_screen.png" << std::endl;
      success = false;
    }
  }
  {
    std::string backgroundPath = resourcesDir + "/background.png";
    std::string outputPath = outputDir + "/background_data.h";

    if (!asset::AssetManager::exportImageAsCode(backgroundPath, outputPath)) {
      std::cerr << "[ERROR] Failed to export background.png" << std::endl;
      success = false;
    }
  }
  {
    std::string playerPath = resourcesDir + "/players.gif";
    std::string outputPath = outputDir + "/players_data.h";

    if (!asset::AssetManager::exportImageAsCode(playerPath, outputPath)) {
      std::cerr << "[ERROR] Failed to export players.gif" << std::endl;
      success = false;
    }
  }
  {
    std::string projectilePath = resourcesDir + "/projectiles.gif";
    std::string outputPath = outputDir + "/projectiles_data.h";

    if (!asset::AssetManager::exportImageAsCode(projectilePath, outputPath)) {
      std::cerr << "[ERROR] Failed to export projectiles.gif" << std::endl;
      success = false;
    }
  }
  {
    std::string enemyPath = resourcesDir + "/enemies.gif";
    std::string outputPath = outputDir + "/enemies_data.h";

    if (!asset::AssetManager::exportImageAsCode(enemyPath, outputPath)) {
      std::cerr << "[ERROR] Failed to export enemies.gif" << std::endl;
      success = false;
    }
  }

  if (success) {
    std::cout << "=== Completed ===" << std::endl;
    return OK;
  } else {
    std::cerr << "=== ERROR ===" << std::endl;
    return KO;
  }
}
