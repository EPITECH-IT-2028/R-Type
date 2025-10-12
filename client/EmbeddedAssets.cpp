#include "EmbeddedAssets.hpp"
#include "AssetManager.hpp"

#include "background_data.h"
#include "players_data.h"
#include "enemies_data.h"

namespace asset {
  void initEmbeddedAssets() {
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

    AssetManager::registerEmbeddedImage(
        "enemy",
        ENEMIES_DATA_DATA,
        ENEMIES_DATA_WIDTH,
        ENEMIES_DATA_HEIGHT,
        ENEMIES_DATA_FORMAT
    );
  }
}  // namespace asset
