#include "EmbeddedAssets.hpp"
#include "AssetManager.hpp"
#include "background_data.h"
#include "enemies_data.h"
#include "players_data.h"
#include "projectiles_data.h"
#include "start_screen_data.h"

namespace asset {
  void initEmbeddedAssets() {
    AssetManager::registerEmbeddedImage(
        "start_screen", START_SCREEN_DATA_DATA, START_SCREEN_DATA_WIDTH,
        START_SCREEN_DATA_HEIGHT, START_SCREEN_DATA_FORMAT);

    AssetManager::registerEmbeddedImage(
        "background", BACKGROUND_DATA_DATA, BACKGROUND_DATA_WIDTH,
        BACKGROUND_DATA_HEIGHT, BACKGROUND_DATA_FORMAT);

    AssetManager::registerEmbeddedImage("players", PLAYERS_DATA_DATA,
                                        PLAYERS_DATA_WIDTH, PLAYERS_DATA_HEIGHT,
                                        PLAYERS_DATA_FORMAT);

    AssetManager::registerEmbeddedImage(
        "projectiles", PROJECTILES_DATA_DATA, PROJECTILES_DATA_WIDTH,
        PROJECTILES_DATA_HEIGHT, PROJECTILES_DATA_FORMAT);

    AssetManager::registerEmbeddedImage("enemy", ENEMIES_DATA_DATA,
                                        ENEMIES_DATA_WIDTH, ENEMIES_DATA_HEIGHT,
                                        ENEMIES_DATA_FORMAT);
  }
}  // namespace asset
