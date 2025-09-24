# R-Type - A Game Engine That Roars! 🚀

[![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.30%2B-green.svg)](https://cmake.org/)
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
- **CMake**: Version 3.20 or higher
- **Package Manager**: Conan 2.0+ (recommended) or vcpkg
- **Git**: For version control

### Installation

1. **Clone the repository**
```bash
git clone git@github.com:EpitechPGE3-2025/G-CPP-500-BDX-5-1-rtype-4.git
cd G-CPP-500-BDX-5-1-rtype-4
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
├── core/                  # Core engine modules
│   └── network/           # Networking utilities (Packet, PacketBuilder, PacketSender)
├── server/                # Server implementation
│   ├── server.properties  # Server configuration
│   ├── src/               # Server source code
│   │   ├── Broadcast.hpp
│   │   ├── Help.cpp/hpp
│   │   ├── Macros.hpp
│   │   ├── main.cpp
│   │   ├── Parser.cpp/hpp
│   │   ├── Server.cpp/hpp
│   │   ├── errors/        # Error handling (ParamsError.hpp)
│   │   └── packets/       # Packet interfaces and handlers
│   │       ├── APacket.hpp
│   │       ├── IPacket.hpp
│   │       ├── PacketFactory.cpp/hpp
│   │       └── PacketHandler.cpp/hpp
│   └── tests/             # Server unit tests
│       ├── test_server.cpp
│       └── html/          # Test coverage reports
|
└── build.sh # Builder for server and client
```

### Design Patterns
- **Entity-Component-System (ECS)**: Flexible game object management
- **Mediator Pattern**: Decoupled system communication
- **Command Pattern**: Input handling and networking
- **Observer Pattern**: Event system

## 📦 Dependencies

- **Raylib** (5.5): Graphics, Audio, Window management
- **Asio** (1.36+): Networking
- **GTest** (1.14+): Testing framework

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
