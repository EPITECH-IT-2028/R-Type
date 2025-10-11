#pragma once

namespace asset {
  /**
   * @brief Initializes all embedded assets by registering them with
   * AssetManager.
   *
   * This function is called automatically at startup by the AssetInitializer.
   * It registers each embedded image with the AssetManager so they
   * can be accessed via the "embedded://" URL scheme.
   */
  void initEmbeddedAssets();
}  // namespace asset
