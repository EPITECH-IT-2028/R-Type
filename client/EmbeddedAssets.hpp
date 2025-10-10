#pragma once

#include "AssetManager.hpp"
#include "background_data.h"
#include "players_data.h"

namespace asset {
  /**
   * @brief Initializes all embedded assets by registering them with
   * AssetManager.
   *
   * This function must be called once at startup before any embedded assets
   * are loaded. It registers each embedded image with the AssetManager so they
   * can be accessed via the "embedded://" URL scheme.
   */
  inline void initEmbeddedAssets() {
    AssetManager::registerEmbeddedImage(
        "background",
        BACKGROUND_DATA_DATA,
        BACKGROUND_DATA_WIDTH,
        BACKGROUND_DATA_HEIGHT,
        BACKGROUND_DATA_FORMAT
    );
    AssetManager::registerEmbeddedImage(
        "players",
        PLAYERS_DATA_DATA,
        PLAYERS_DATA_WIDTH,
        PLAYERS_DATA_HEIGHT,
        PLAYERS_DATA_FORMAT
    );
  }
}  // namespace asset
