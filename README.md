# R-Type - A Game Engine That Roars! 🚀

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-green.svg)](https://cmake.org/)
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
git clone https://github.com/yourusername/R-Type.git
cd R-Type
```

2. **Install dependencies using Conan**
```bash
brew install conan  # macOS
sudo apt install conan  # Ubuntu
sudo dnf install conan  # Fedora
pip install conan  # Windows (via pip)

conan profile new default --detect # Create a default profile

conan install . --output-folder=build --build=missing
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

## 🎮 How to Play

### Controls
- **Arrow Keys**: Move your spaceship
- **Space**: Fire missiles
- **Shift**: Charge weapon
- **Ctrl**: Deploy/Recall Force module
- **ESC**: Pause menu

### Gameplay
- Fight waves of Bydo enemies
- Collect power-ups to upgrade your weapons
- Coordinate with other players in multiplayer mode
- Survive boss battles at the end of each level

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

## 🌐 Network Protocol

Our custom binary protocol uses UDP for real-time game communication:

- **Packet Types**: Movement, Actions, Game State Updates, Entity Spawning
- **Optimization**: Delta compression, bit packing, interpolation
- **Reliability**: Critical messages use acknowledgment system
- **Security**: Input validation, buffer overflow protection

For detailed protocol documentation, see [docs/protocol/README.md](docs/protocol/README.md)

## 📚 Documentation

- **[Developer Guide](docs/developer/guide.md)**: Architecture overview and development setup
- **[API Reference](docs/api/index.html)**: Complete API documentation
- **[Network Protocol](docs/protocol/rfc.md)**: Detailed protocol specification
- **[Contributing Guide](CONTRIBUTING.md)**: How to contribute to the project

## 🛠️ Development

### Building from Source

**Linux/MacOs:**
```bash
./build.sh server/client
```

**Windows (Visual Studio):**
```bash
.\scripts\build_windows.bat
```

### Running Tests
```bash
cmake --build build --target test
```

### Code Style
We use clang-format for consistent code formatting:
```bash
clang-format -i src/**/*.cpp include/**/*.hpp
```

## 🔧 Configuration

### Server Configuration
Create a `server.properties` file:
```text
PORT=4242
MAX_PLAYERS=4
```

### Client Configuration
Create a `client.config` file:
```json
{
  "resolution": "1920x1080",
  "fullscreen": false,
  "vsync": true,
  "audio_volume": 80
}
```

## ♿ Accessibility

We are committed to making our game accessible to all players:

- **Visual**: Colorblind modes, adjustable UI scale, high contrast options
- **Audio**: Subtitles, visual sound indicators, adjustable audio channels
- **Motor**: Customizable controls, one-handed mode, adjustable difficulty
- **Cognitive**: Clear tutorials, simplified UI option, pause anywhere

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details on:
- Code of conduct
- Development workflow
- Pull request process
- Coding standards

## 📦 Dependencies

- **Raylib** (5.5): Graphics, Audio, Window management
- **Asio** (1.36+): Networking
- **GTest** (1.14+): Testing framework

## 🚦 CI/CD

- **GitHub Actions**: Automated builds and tests
- **Code Coverage**: Codecov integration
- **Static Analysis**: clang-tidy, cppcheck
- **Deployment**: Docker containers for server

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👥 Team

- **Lead Developer**: Nolann DUBOS
- **Network Engineer**: Arthur Guerinault
- **Game Designer**: Etienne LABARBE, Quentin LAPIERRE, Robin CHASSAGNE

## 🙏 Acknowledgments

- Original R-Type by Irem
- Raylib community
- Modern C++ community

## 📮 Contact

- TO DO **Discord**: [Join our server](https://discord.gg/rtype)
- TO DO **Email**: team@rtype-project.com
- TO DO **Issues**: [GitHub Issues](https://github.com/yourusername/r-type/issues)

## 🔗 Links

- TO DO [Project Website](https://rtype-project.com)
- TO DO [Documentation](https://docs.rtype-project.com)
- TO DO [Wiki](https://github.com/yourusername/r-type/wiki)
- [Original R-Type Info](https://en.wikipedia.org/wiki/R-Type)

---

<div align="center">
  <b>R-Type - A Game Engine That Roars!</b><br>
  Made with ❤️ and C++
</div>

---

## Tests
To run tests:

```sh
cmake -B .build -DENABLE_TESTS=ON -DCMAKE_CXX_FLAGS="--coverage"
cmake --build .build
./unit_tests
```

```sh
gcovr -r . --html --html-details -o tests/html/coverage.html
# macOS
open tests/html/coverage.html
# Linux
xdg-open tests/html/coverage.html || true
# Windows (PowerShell)
start tests/html/coverage.html
```
