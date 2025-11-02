# R-Type - A Game Engine That Roars! 🚀

[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.27.4%2B-green.svg)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey.svg)](https://github.com/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## 📖 Overview

R-Type is a modern reimplementation of the classic horizontal shoot'em up game, featuring a **networked multiplayer architecture** and a **custom game engine**. This project demonstrates advanced C++ development techniques, proper software engineering practices, and real-time networked game development.

### Key Features

- 🎮 **Multiplayer Support**: Up to 4 players can fight together against the evil Bydo forces
- 🌐 **Client-Server Architecture**: Authoritative server with multiple client support
- 🎯 **Custom Game Engine**: Modular, extensible architecture with ECS (Entity-Component-System) design
- 🔧 **Cross-Platform**: Runs on both Linux and Windows
- 📡 **UDP Networking**: Fast, real-time communication with custom binary protocol
- 🎨 **Raylib Graphics**: Smooth rendering with particle effects and animations

## 🚀 Quick Start

### Prerequisites

- **C++ Compiler**: GCC 9+ or MSVC 2019+
- **CMake**: Version 3.27.4
- **Package Manager**: Conan 2.0+ (recommended) or vcpkg
- **Git**: For version control

### Installation

1. **Repository's URL**

```bash
https://github.com/EpitechPGE3-2025/G-CPP-500-BDX-5-1-rtype-4.git
```

2. **Install dependencies using Conan**

```bash
brew install conan  # macOS
sudo apt install conan  # Ubuntu
sudo dnf install conan  # Fedora
pip install conan  # Windows (via pip)
```

3. **Build the project**

```bash
./build.sh server/client # For Linux/MacOS
```

4. **Run the server**

```bash
./r-type_server
```

5. **Run the client**

```bash
./r-type_client <name>
```

## 🏗️ Architecture

### Project Structure

```
R-Type/
├── CMakeLists.txt                # Main CMake configuration
├── CMakeUserPresets.json         # CMake presets for builds
├── conanfile.txt                 # Conan dependencies (Client)
├── conanfile_server.txt          # Conan dependencies (Server)
├── build.sh                      # Build script for server and client
├── README_BUILD.md               # Detailed build instructions
├── TECHNICAL_COMPARATIVE_STUDY.md # Technical documentation
├── compile_commands.json         # Compilation database for IDEs
├── db.sql                        # Database schema
├── r_type_client                 # Client executable
├── r_type_server                 # Server executable
│
├── admin/                        # Admin panel for database management
│   ├── main.py                   # Admin interface entry point
│   ├── database.py               # Database connection handler
│   ├── requirements.txt          # Python dependencies
│   └── textuals/                 # Textual UI components
│       ├── __init__.py
│       ├── bans_management.py
│       ├── players_management.py
│       └── scores_management.py
│
├── client/                       # Client implementation
│   ├── CMakeLists.txt
│   ├── main.cpp                  # Client entry point
│   ├── client.properties         # Client configuration
│   ├── Client.cpp/hpp            # Client logic
│   ├── AssetManager.cpp/hpp      # Asset loading and management
│   ├── RenderManager.cpp/hpp     # Rendering system
│   ├── EmbeddedAssets.cpp/hpp    # Embedded resources
│   ├── EmbedAssets.cpp           # Asset embedding
│   ├── Challenge.hpp             # Challenge system
│   ├── ProjectileSpriteConfig.hpp # Projectile sprite configuration
│   ├── packets/                  # Client packet handling
│   │   ├── APacket.hpp
│   │   ├── IPacket.hpp
│   │   ├── PacketFactory.cpp/hpp
│   │   └── PacketHandler.cpp/hpp
│   └── resources/                # Game resources (sprites, sounds, etc.)
│
├── core/                         # Shared core modules
│   ├── Parser.cpp/hpp            # Configuration file parser
│   ├── errors/                   # Error handling
│   │   └── ParamsError.hpp
│   ├── network/                  # Network protocol definitions
│   │   ├── Packet.hpp            # Packet types and enums
│   │   ├── PacketBuilder.hpp     # Packet construction
│   │   ├── PacketSender.hpp      # Packet sending utilities
│   │   ├── PacketSerialize.hpp   # Packet serialization
│   │   ├── PacketUtils.hpp       # Packet utilities
│   │   ├── Serializer.hpp        # Generic serialization
│   │   ├── BaseNetworkManager.hpp # Base network manager
│   │   ├── ClientNetworkManager.cpp/hpp # Client network manager
│   │   └── ServerNetworkManager.cpp/hpp # Server network manager
│   └── utils/                    # Utility functions
│       ├── Crypto.hpp            # Cryptographic utilities
│       ├── Macro.hpp             # Common macros
│       ├── RandomNameGenerator.hpp # Random name generation
│       └── RaylibUtils.hpp       # Raylib helper functions
│
├── docs/                         # Documentation
│   ├── server_how_to.md          # Server usage guide
│   ├── server_protocol.md        # Network protocol documentation
│   ├── doxygen/                  # Generated Doxygen documentation
│   └── wiki/                     # Project wiki
│       ├── AssetManagement.md
│       ├── ContributionGuidelines.md
│       ├── GameEngineECS.md
│       ├── GettingStarted.md
│       ├── Home.md
│       ├── Networking.md
│       └── ProjectStructure.md
│
├── game_engine/                  # Custom ECS game engine
│   └── ecs/                      # Entity-Component-System
│       ├── Component.hpp
│       ├── ComponentManager.hpp
│       ├── ECSManager.hpp        # Main ECS coordinator
│       ├── EntityManager.cpp/hpp
│       ├── System.hpp
│       ├── SystemManager.hpp
│       ├── components/           # Game components
│       │   ├── ChatComponent.hpp
│       │   ├── ColliderComponent.hpp
│       │   ├── EnemyComponent.hpp
│       │   ├── HealthComponent.hpp
│       │   ├── PlayerComponent.hpp
│       │   ├── PositionComponent.hpp
│       │   ├── ProjectileComponent.hpp
│       │   ├── RenderComponent.hpp
│       │   ├── ScaleComponent.hpp
│       │   ├── ScoreComponent.hpp
│       │   ├── ShootComponent.hpp
│       │   ├── SpeedComponent.hpp
│       │   ├── SpriteAnimationComponent.hpp
│       │   ├── SpriteComponent.hpp
│       │   └── VelocityComponent.hpp
│       ├── systems/              # Game systems
│       │   ├── BackgroundSystem.cpp/hpp
│       │   ├── CollisionSystem.cpp/hpp
│       │   ├── EnemySystem.cpp/hpp
│       │   ├── InputSystem.cpp/hpp
│       │   ├── MovementSystem.cpp/hpp
│       │   ├── ProjectileSystem.cpp/hpp
│       │   ├── RenderSystem.cpp/hpp
│       │   ├── ServerInputSystem.cpp/hpp
│       │   └── SpriteAnimationSystem.cpp/hpp
│       └── tags/                 # Entity tags
│           ├── BackgroundTagComponent.hpp
│           ├── LocalPlayerTagComponent.hpp
│           └── PlayerTagComponent.hpp
│
└── server/                       # Server implementation
    ├── CMakeLists.txt
    ├── server.properties         # Server configuration file
    └── src/                      # Server source code
        ├── main.cpp              # Server entry point with ASIO
        ├── Server.cpp/hpp        # Main server class
        ├── Client.cpp/hpp        # Client connection handler
        ├── DatabaseManager.cpp/hpp # Database operations
        ├── Help.cpp/hpp          # Help system
        ├── Broadcast.hpp         # Network broadcasting utilities
        ├── enemy/                # Enemy management
        │   └── Enemy.cpp/hpp
        ├── game/                 # Game logic
        │   ├── Game.cpp/hpp
        │   ├── GameManager.cpp/hpp
        │   ├── GameRoom.hpp
        │   └── Challenge.cpp/hpp
        ├── packets/              # Network packet handling
        │   ├── APacket.hpp
        │   ├── IPacket.hpp
        │   ├── PacketFactory.cpp/hpp
        │   └── PacketHandler.cpp/hpp
        ├── player/               # Player management
        │   └── Player.cpp/hpp
        ├── projectile/           # Projectile management
        │   └── Projectile.cpp/hpp
        └── queue/                # Event queue system
            ├── Events.hpp
            └── Queue.hpp
```

### Design Patterns

- **Entity-Component-System (ECS)**: Flexible game object management
- **Mediator Pattern**: Decoupled system communication
- **Command Pattern**: Input handling and networking
- **Observer Pattern**: Event system

## 📦 Dependencies

- **Raylib** (5.5): Graphics, Audio, Window management
- **Asio** (1.29.0): Networking
- **GTest** (1.17.0): Testing framework

## 👥 Team

- **Lead Developer**: Nolann DUBOS
- **Network Engineer**: Arthur Guerinault
- **Game Designer**: Etienne LABARBE, Quentin LAPIERRE, Robin CHASSAGNE

## 📚 Documentation

📚 **For detailed documentation, visit our [GitHub Wiki](https://epitech-it-2028.github.io/R-Type/wiki)**

## 🙏 Acknowledgments

- Original R-Type by Irem
- Raylib community
- Modern C++ community

---

<div align="center">
  <b>R-Type - A Game Engine That Roars!</b><br>
  Made with ❤️ and C++
</div>
