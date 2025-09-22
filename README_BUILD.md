# R-Type Build System

This project uses CMake with Conan for dependency management. You can build either the client, server, or both.

## Prerequisites

- CMake 3.27.4 or higher
- Conan 2.x
- C++20 compatible compiler
- Git (for ASIO dependency)

## Quick Start

## Build Script Usage

The build script automatically handles dependency installation and build configuration. Here are the supported usage patterns:

### No arguments (default)
```bash
./build.sh
```
Builds both client and server in Release mode.

### Single argument - Build Type
```bash
./build.sh debug    # Builds both client and server in Debug mode
./build.sh release  # Builds both client and server in Release mode
```

### Single argument - Target
```bash
./build.sh client   # Builds client only in Release mode
./build.sh server   # Builds server only in Release mode
./build.sh both     # Builds both client and server in Release mode
```

### Single argument - Clean
```bash
./build.sh clean    # Removes all build files and directories
```
This will remove:
- `.build/` directory
- `build/` directory (if it exists)
- `CMakeCache.txt` file (if it exists)
- `CMakeFiles/` directory (if it exists)

### Two arguments - Any order
```bash
# Target first, build type second
./build.sh client debug
./build.sh server release
./build.sh both debug

# Build type first, target second
./build.sh debug client
./build.sh release server
./build.sh debug both
```

**Note:** The script automatically cleans the build directory and reinstalls dependencies on each run to ensure a clean build.

### Manual build:

#### Building the Client

```bash
conan install . --output-folder=.build --build=missing
cmake -B .build -DCMAKE_TOOLCHAIN_FILE=".build/conan_toolchain.cmake" -DBUILD_CLIENT=ON -DBUILD_SERVER=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build .build --config Release
```

#### Building the Server

```bash
conan install . --output-folder=.build --build=missing
cmake -B .build -DCMAKE_TOOLCHAIN_FILE=".build/conan_toolchain.cmake" -DBUILD_CLIENT=OFF -DBUILD_SERVER=ON -DCMAKE_BUILD_TYPE=Release
cmake --build .build --config Release
```

#### Building Both

```bash
conan install . --output-folder=.build --build=missing
cmake -B .build -DCMAKE_TOOLCHAIN_FILE=".build/conan_toolchain.cmake" -DBUILD_CLIENT=ON -DBUILD_SERVER=ON -DCMAKE_BUILD_TYPE=Release
cmake --build .build --config Release
```

## Dependencies

- **Client**: raylib (managed by Conan)
- **Server**: asio (managed by Conan)

## Output

Executables will be created at the root of the project:
- `r_type_client` - The game client
- `r_type_server` - The game server

## Debug Builds

For debug builds, use:
```bash
./build.sh debug
```

Or manually:
```bash
conan install . --output-folder=.build --build=missing
cmake -B .build -DCMAKE_TOOLCHAIN_FILE=".build/conan_toolchain.cmake" -DBUILD_CLIENT=ON -DBUILD_SERVER=ON -DCMAKE_BUILD_TYPE=Release
cmake --build .build --config Debug
```

## CMake Options

- `BUILD_CLIENT`: Build the client (OFF by default)
- `BUILD_SERVER`: Build the server (OFF by default)
- `CMAKE_BUILD_TYPE`: Release or Debug (Release by default)

At least one of `BUILD_CLIENT` or `BUILD_SERVER` must be ON.

