# Project Structure

This page details the overall directory and file structure of the R-Type project, explaining the purpose of each major component.

```
R-Type/
├───.github/                  # GitHub Actions workflows for CI/CD
├───.venv/                    # Python virtual environment (likely for Conan or other tooling)
├───.venv-docker/             # Docker-related Python virtual environment
├───client/                   # Client-side application source code and assets
│   ├───AssetManager.cpp/.hpp # Manages game assets (sprites, textures)
│   ├───Client.cpp/.hpp       # Main client application logic
│   ├───client.properties     # Configuration file for the client
│   ├───CMakeLists.txt        # CMake build script for the client
│   ├───EmbedAssets.cpp       # Utility to embed assets into the executable
│   ├───EmbeddedAssets.cpp/.hpp # Embedded asset data
│   ├───main.cpp              # Client entry point
│   ├───ProjectileSpriteConfig.hpp # Configuration for projectile sprites
│   ├───RenderManager.cpp/.hpp # Handles all rendering operations
│   └───packets/              # Client-side packet handling
│       ├───APacket.hpp       # Abstract base class for packets
│       ├───IPacket.hpp       # Interface for packets
│       ├───PacketFactory.cpp/.hpp # Factory for creating packets
│       ├───PacketHandler.cpp/.hpp # Handles incoming and outgoing packets
│   └───resources/            # Game assets (images, spritesheets)
├───core/                     # Core shared utilities and networking components
│   ├───Parser.cpp/.hpp       # Utility for parsing (e.g., configuration files)
│   ├───errors/               # Custom error classes
│   │   └───ParamsError.hpp   # Error for invalid parameters
│   └───network/              # Core networking components
│       ├───BaseNetworkManager.hpp # Base class for network managers
│       ├───ClientNetworkManager.cpp/.hpp # Client-specific network management
│       ├───Packet.hpp        # Defines the structure of network packets
│       ├───PacketBuilder.hpp # Utility for building packets
│       ├───PacketSender.hpp  # Handles sending packets
│       ├───ServerNetworkManager.cpp/.hpp # Server-specific network management
│   └───utils/                # General utility functions/macros
│       └───Macro.hpp         # Common macros
├───docs/                     # Project documentation
│   ├───server_how_to.md      # Guide for setting up the server
│   ├───server_protocol.md    # Server communication protocol specification
│   └───wiki/                 # GitHub Wiki markdown files (this directory)
├───game_engine/              # Core Entity-Component-System (ECS) implementation
│   └───ecs/                  # ECS framework
│       ├───Component.hpp     # Base class for components
│       ├───ComponentManager.hpp # Manages components
│       ├───ECSManager.hpp    # Main ECS manager
│       ├───EntityManager.cpp/.hpp # Manages entities
│       ├───System.hpp        # Base class for systems
│       ├───SystemManager.hpp # Manages systems
│       ├───components/       # Specific component implementations
│       │   ├───ColliderComponent.hpp
│       │   ├───EnemyComponent.hpp
│       │   ├───HealthComponent.hpp
│       │   ├───PlayerComponent.hpp
│       │   ├───PositionComponent.hpp
│       │   ├───ProjectileComponent.hpp
│       │   ├───RenderComponent.hpp
│       │   ├───ScaleComponent.hpp
│       │   ├───ScoreComponent.hpp
│       │   ├───ShootComponent.hpp
│       │   ├───SpeedComponent.hpp
│       │   ├───SpriteAnimationComponent.hpp
│       │   ├───SpriteComponent.hpp
│       │   └───VelocityComponent.hpp
│       ├───systems/          # Specific system implementations
│       │   ├───BackgroundSystem.cpp/.hpp
│       │   ├───BoundarySystem.cpp/.hpp
│       │   ├───CollisionSystem.cpp/.hpp
│       │   ├───EnemySystem.cpp/.hpp
│       │   ├───InputSystem.cpp/.hpp
│       │   ├───MovementSystem.cpp/.hpp
│       │   ├───ProjectileSystem.cpp/.hpp
│       │   ├───RenderSystem.cpp/.hpp
│       │   ├───SpriteAnimationSystem.cpp/.hpp
│       └───tags/             # Tag components (components without data)
│           ├───BackgroundTagComponent.hpp
│           ├───LocalPlayerTagComponent.hpp
│           └───PlayerTagComponent.hpp
├───server/                   # Server-side application source code
│   ├───CMakeLists.txt        # CMake build script for the server
│   ├───server.properties     # Configuration file for the server
│   └───src/                  # Server source files
│       ├───Broadcast.hpp     # Utility for broadcasting messages
│       ├───Help.cpp/.hpp     # Help command implementation
│       ├───main.cpp          # Server entry point
│       ├───Server.cpp/.hpp   # Main server application logic
│       ├───enemy/            # Enemy-related logic
│       │   ├───Enemy.cpp/.hpp
│       ├───game/             # Game logic and state management
│       │   ├───Game.cpp/.hpp
│       ├───packets/          # Server-side packet handling (similar to client)
│       │   ├───APacket.hpp
│       │   ├───IPacket.hpp
│       │   ├───PacketFactory.cpp/.hpp
│       │   ├───PacketHandler.cpp/.hpp
│       ├───player/           # Player-related logic
│       │   ├───Player.cpp/.hpp
│       ├───projectile/       # Projectile-related logic
│       │   ├───Projectile.cpp/.hpp
│       └───queue/            # Event queue for server
│           ├───Events.hpp
│           └───Queue.hpp
│   └───tests/                # Server unit tests
├───.gitignore                # Git ignore rules
├───build.sh                  # Build script
├───CMakeLists.txt            # Main CMake project file
├───conanfile_server.txt      # Conan dependencies for the server
├───conanfile.txt             # Main Conan dependencies
├───README_BUILD.md           # Build instructions
├───README.md                 # Main project README
└───TECHNICAL_COMPARATIVE_STUDY.md # Technical study document
```

## Main Directories Explained

*   **`.github/`**: Contains GitHub Actions workflows for continuous integration and deployment.
*   **`client/`**: Houses all the code and resources specific to the R-Type client application. This includes rendering logic, asset management, client-side networking, and the main client application entry point.
*   **`core/`**: This directory contains shared code that is utilized by both the client and the server. It includes common utilities, error handling, and the foundational networking components.
*   **`docs/`**: Stores project documentation, including detailed server setup guides, network protocol specifications, and the GitHub Wiki markdown files.
*   **`game_engine/`**: This is where the core Entity-Component-System (ECS) framework is implemented. It defines the base classes and managers for entities, components, and systems, along with specific implementations for various game mechanics.
*   **`server/`**: Contains all the code and logic for the R-Type game server. This includes game state management, server-side networking, and specific game entities like enemies, players, and projectiles.
*   **`.venv/` & `.venv-docker/`**: These directories typically contain Python virtual environments, likely used for managing development tools, Conan, or Docker-related scripting.
*   **Root Files**:
    *   `build.sh`: A shell script to automate the build process.
    *   `CMakeLists.txt`: The primary CMake configuration file for the entire project.
    *   `conanfile.txt` & `conanfile_server.txt`: Conan recipe files specifying project dependencies.
    *   `README.md`, `README_BUILD.md`, `TECHNICAL_COMPARATIVE_STUDY.md`: Project documentation and build instructions.