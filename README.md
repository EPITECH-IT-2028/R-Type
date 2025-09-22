# R-Type - A Game Engine That Roars! 🚀

[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.27.4%2B-green.svg)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey.svg)](https://github.com/)

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
./r-type_client
```

## 🏗️ Architecture

### Project Structure
```
R-Type/
├── CMakeLists.txt         # Main CMake configuration
├── CMakeUserPresets.json  # CMake presets for builds
├── conanfile.txt          # Conan dependencies (Raylib, ASIO)
├── build.sh               # Build script for server and client
├── README_BUILD.md        # Detailed build instructions
├── compile_commands.json  # Compilation database for IDEs
│
├── client/                # Client implementation
│   ├── CMakeLists.txt
│   └── main.cpp           # Raylib-based client entry point
│
├── core/                  # Shared core modules
│   └── network/           # Network protocol definitions
│       ├── Packet.hpp     # Packet types and enums
│       ├── PacketBuilder.hpp
│       └── PacketSender.hpp
│
├── game_engine/           # Custom ECS game engine
│   └── ecs/               # Entity-Component-System
│       ├── Component.hpp
│       ├── ComponentManager.hpp
│       ├── ECSManager.hpp # Main ECS coordinator
│       ├── EntityManager.cpp/hpp
│       ├── System.hpp
│       ├── SystemManager.hpp
│       ├── components/    # Game components
│       │   ├── HealthComponent.hpp
│       │   ├── PlayerComponent.hpp
│       │   ├── PositionComponent.hpp
│       │   ├── SpeedComponent.hpp
│       │   └── VelocityComponent.hpp
│       └── systems/       # Game systems (to be implemented)
│
└── server/                # Server implementation
    ├── CMakeLists.txt
    ├── server.properties  # Server configuration file
    ├── src/               # Server source code
    │   ├── main.cpp       # Server entry point with ASIO
    │   ├── Server.cpp/hpp # Main server class
    │   ├── Parser.cpp/hpp # Config file parser
    │   ├── Help.cpp/hpp   # Help system
    │   ├── Broadcast.hpp  # Network broadcasting utilities
    │   ├── Macros.hpp     # Common macros
    │   ├── errors/        # Error handling
    │   │   └── ParamsError.hpp
    │   ├── game/          # Game logic
    │   │   ├── Game.cpp/hpp
    │   │   └── Player.cpp/hpp
    │   └── packets/       # Network packet handling
    │       ├── APacket.hpp
    │       ├── IPacket.hpp
    │       ├── PacketFactory.cpp/hpp
    │       └── PacketHandler.cpp/hpp
    └── tests/             # Server unit tests
        ├── test_server.cpp
        └── html/          # Test coverage reports
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

## 🙏 Acknowledgments

- Original R-Type by Irem
- Raylib community
- Modern C++ community

---

<div align="center">
  <b>R-Type - A Game Engine That Roars!</b><br>
  Made with ❤️ and C++
</div>
