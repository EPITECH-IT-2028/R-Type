# R-Type Build System

This project uses CMake with Conan for dependency management. You can build either the client, server, or both.

## Prerequisites

- CMake 3.16 or higher
- Conan 2.x
- C++20 compatible compiler
- Git (for ASIO dependency)

## Quick Start

### Using the build script (recommended):

```bash
# Build everything
./build.sh both

# Build only client
./build.sh client

# Build only server
./build.sh server

# Build with debug configuration
./build.sh both debug
```

### Manual build:

#### Building the Client

```bash
mkdir build && cd build
conan install .. --build=missing -s build_type=Release
cmake .. -DBUILD_CLIENT=ON -DBUILD_SERVER=OFF
cmake --build .
```

#### Building the Server

```bash
mkdir build && cd build
cmake .. -DBUILD_CLIENT=OFF -DBUILD_SERVER=ON
cmake --build .
```

#### Building Both

```bash
mkdir build && cd build
conan install .. --build=missing -s build_type=Release
cmake .. -DBUILD_CLIENT=ON -DBUILD_SERVER=ON
cmake --build .
```

## Dependencies

- **Client**: raylib (managed by Conan)
- **Server**: ASIO (fetched automatically via CMake FetchContent)

## Output

Executables will be created in `build/bin/`:
- `r_type_client` - The game client
- `r_type_server` - The game server

## Debug Builds

For debug builds, use:
```bash
./build.sh both debug
```

Or manually:
```bash
cmake .. -DBUILD_CLIENT=ON -DBUILD_SERVER=ON -DCMAKE_BUILD_TYPE=Debug
```

## CMake Options

- `BUILD_CLIENT`: Build the client (OFF by default)
- `BUILD_SERVER`: Build the server (OFF by default)
- `CMAKE_BUILD_TYPE`: Release or Debug (Release by default)

At least one of `BUILD_CLIENT` or `BUILD_SERVER` must be ON.

