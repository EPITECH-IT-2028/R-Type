# Asset Management

This section details how game assets (sprites, backgrounds, etc.) are managed and loaded within the R-Type project, primarily focusing on the client-side implementation.

## Overview

The R-Type client employs an `AssetManager` to handle the loading and management of various game assets, such as textures and images. A key feature of this system is the ability to embed assets directly into the executable at compile-time, reducing reliance on external files at runtime. This is particularly useful for ensuring all necessary assets are present and loaded efficiently.

## `AssetManager` (`client/AssetManager.hpp`)

The `asset::AssetManager` class is responsible for:

*   **Embedded Asset Handling**: It can export image files as C header files (`exportImageAsCode`) which contain the image data as static arrays. These embedded assets can then be registered (`registerEmbeddedImage`) and loaded at runtime using a special `embedded://` scheme.
*   **Runtime Loading**: Provides methods (`loadTexture`, `loadImage`) to load assets. It first attempts to load from embedded data; if not found or if the path doesn't use the `embedded://` scheme, it falls back to loading from the file system.
*   **Texture and Image Management**: Utilizes `raylib` functions for handling `Texture2D` and `Image` types.

### Key Methods:

*   `static bool exportImageAsCode(const std::string &imagePath, const std::string &outputHeaderPath)`: Exports an image to a C header file for embedding.
*   `static Texture2D loadTexture(const std::string &path)`: Loads a texture, prioritizing embedded assets.
*   `static Image loadImage(const std::string &path)`: Loads an image, prioritizing embedded assets.
*   `static void registerEmbeddedImage(const std::string &name, void *data, int width, int height, int format)`: Registers an embedded image with a given name and its properties.

## `RenderManager` (`client/RenderManager.hpp`)

The `renderManager::Renderer` class, while primarily focused on rendering operations, defines constants for the paths to various core game assets. These paths often point to embedded assets, indicating their importance and how they are accessed by the rendering system.

### Asset Paths Defined:

*   `WINDOW_WIDTH`, `WINDOW_HEIGHT`: Window dimensions.
*   `SCROLL_SPEED`: Speed for background scrolling.
*   `BG_PATH`: Path to the background asset (e.g., `embedded://background`).
*   `PLAYER_PATH`: Path to the player spritesheet (e.g., `embedded://players`).
*   `PROJECTILE_PATH`: Path to the projectile spritesheet (e.g., `embedded://projectiles`).
*   `ENEMY_PATH`: Path to the enemy spritesheet (e.g., `embedded://enemy`).

## Asset Resources (`client/resources/`)

This directory contains the raw image files that are used as game assets. These files are typically processed by the `AssetManager` (e.g., embedded) or loaded directly at runtime.

### Contents:

*   `background.png`: The main background image for the game.
*   `enemies.gif`: Spritesheet or animation for enemy characters.
*   `players.gif`: Spritesheet or animation for player characters.
*   `projectiles.gif`: Spritesheet or animation for projectiles.
*   `start_screen.png`: Image for the game's start screen.

By combining embedded assets with a fallback to file-system loading, the R-Type client ensures efficient asset delivery and flexibility during development.