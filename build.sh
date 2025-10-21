#!/bin/bash

# Usage: ./build.sh [debug|release] (defaults to both)
#        ./build.sh [client|server|both] [debug|release]
#        ./build.sh clean

set -e

REQUIRED_PACKAGES=(
    build-essential
    cmake
    ninja-build
    pkg-config
    libgl-dev
    libgl1-mesa-dev
    libx11-dev
    libx11-xcb-dev
    libfontenc-dev
    libice-dev
    libsm-dev
    libxau-dev
    libxaw7-dev
    libxcomposite-dev
    libxcursor-dev
    libxdamage-dev
    libxext-dev
    libxfixes-dev
    libxi-dev
    libxinerama-dev
    libxkbfile-dev
    libxmu-dev
    libxmuu-dev
    libxpm-dev
    libxrandr-dev
    libxrender-dev
    libxres-dev
    libxss-dev
    libxt-dev
    libxtst-dev
    libxv-dev
    libxxf86vm-dev
    libxcb-glx0-dev
    libxcb-render0-dev
    libxcb-render-util0-dev
    libxcb-xkb-dev
    libxcb-icccm4-dev
    libxcb-image0-dev
    libxcb-keysyms1-dev
    libxcb-randr0-dev
    libxcb-shape0-dev
    libxcb-sync-dev
    libxcb-xfixes0-dev
    libxcb-xinerama0-dev
    uuid-dev
    libxcb-cursor-dev
    libxcb-dri2-0-dev
    libxcb-dri3-dev
    libxcb-present-dev
    libxcb-composite0-dev
    libxcb-ewmh-dev
    libxcb-res0-dev
    libxcb-util-dev
    libxcb-util0-dev
    clang
)

UNAME_OUT="$(uname -s)"
case "${UNAME_OUT}" in
    Linux*)
        OS_TYPE=Linux
        ;;
    Darwin*)
        OS_TYPE=Mac
        ;;
    CYGWIN*|MINGW*|MSYS*)
        OS_TYPE=Windows
        ;;
    *)
        OS_TYPE="Unknown"
        ;;
esac

if [[ "$OS_TYPE" == "Linux" ]]; then
    echo "Linux detected, ensuring required packages are installed..."
    if [[ $EUID -ne 0 ]]; then
       echo "Some dependencies may require root privileges. Please run as root (sudo ./build.sh) if packages are missing."
    fi
    for pkg in "${REQUIRED_PACKAGES[@]}"; do
        if ! dpkg -s "$pkg" &> /dev/null; then
            echo "[...] Installing $pkg"
            sudo apt-get update
            sudo apt-get install -y "$pkg"
        fi
    done
else
    echo "Non-Linux OS detected ($OS_TYPE), skipping system package installation."
fi

if [ $# -eq 0 ]; then
    TARGET="both"
    BUILD_TYPE="Release"
elif [ $# -eq 1 ]; then
    case $1 in
        "clean"|"Clean"|"CLEAN")
            echo -e "\033[1;33m[ OK ] Cleaning all build files and folders...\033[0m"
            if [ -d ".build" ]; then
                echo -e "\033[1;31m[ OK ] Removing .build directory...\033[0m"
                rm -rf .build
            fi
            if [ -d "build" ]; then
                echo -e "\033[1;31m[ OK ] Removing build directory...\033[0m"
                rm -rf build
            fi
            if [ -f "CMakeCache.txt" ]; then
                echo -e "\033[1;31m[ OK ] Removing CMakeCache.txt...\033[0m"
                rm -f CMakeCache.txt
            fi
            if [ -d "CMakeFiles" ]; then
                echo -e "\033[1;31m[ OK ] Removing CMakeFiles directory...\033[0m"
                rm -rf CMakeFiles
            fi
            if [ -f "CMakeUserPresets.json" ]; then
                echo -e "\033[1;31m[ OK ] Removing CMakeUserPresets.json...\033[0m"
                rm -rf CMakeUserPresets.json
            fi
            if [ -L "compile_commands.json" ] || [ -f "compile_commands.json" ]; then
                echo -e "\033[1;31m[ OK ] Removing compile_commands.json...\033[0m"
                rm -f compile_commands.json
            fi
            if [ -f "r_type_client" ]; then
                echo -e "\033[1;31m[ OK ] Removing r_type_client executable...\033[0m"
                rm -f r_type_client
            fi
            if [ -f "r_type_server" ]; then
                echo -e "\033[1;31m[ OK ] Removing r_type_server executable...\033[0m"
                rm -f r_type_server
            fi
            if [ -f "embed_assets" ]; then
                echo -e "\033[1;31m[ OK ] Removing embed_assets executable...\033[0m"
                rm -f embed_assets
            fi
            echo -e "\033[1;32m[ OK ] Clean completed successfully!\033[0m"
            exit 0
            ;;
        "debug"|"Debug"|"DEBUG")
            TARGET="both"
            BUILD_TYPE="Debug"
            ;;
        "release"|"Release"|"RELEASE")
            TARGET="both"
            BUILD_TYPE="Release"
            ;;
        "client"|"server"|"both")
            TARGET="$1"
            BUILD_TYPE="Release"
            ;;
        *)
            echo "Invalid argument. Use: client, server, both, debug, release, or clean"
            exit 1
            ;;
    esac
elif [ $# -eq 2 ]; then
    case $1 in
        "debug"|"Debug"|"DEBUG")
            BUILD_TYPE="Debug"
            case $2 in
                "client"|"server"|"both")
                    TARGET="$2"
                    ;;
                *)
                    echo "Invalid target. Use: client, server, or both"
                    exit 1
                    ;;
            esac
            ;;
        "release"|"Release"|"RELEASE")
            BUILD_TYPE="Release"
            case $2 in
                "client"|"server"|"both")
                    TARGET="$2"
                    ;;
                *)
                    echo "Invalid target. Use: client, server, or both"
                    exit 1
                    ;;
            esac
            ;;
        "client"|"server"|"both")
            TARGET="$1"
            case $2 in
                "debug"|"Debug"|"DEBUG")
                    BUILD_TYPE="Debug"
                    ;;
                "release"|"Release"|"RELEASE")
                    BUILD_TYPE="Release"
                    ;;
                *)
                    echo "Invalid build type. Use: debug or release"
                    exit 1
                    ;;
            esac
            ;;
        *)
            echo "Invalid first argument. Use: client, server, both, debug, or release"
            exit 1
            ;;
    esac
else
    echo "Too many arguments. Usage: ./build.sh [client|server|both] [debug|release] or ./build.sh [debug|release] [client|server|both]"
    exit 1
fi

echo "Building R-Type with target: $TARGET, build type: $BUILD_TYPE"

echo "Installing conan dependencies..."
if ! conan profile show > /dev/null 2>&1; then
    echo "[INFO] Conan default profile not found. Detecting..."
    conan profile detect --force > /dev/null 2>&1
fi

CONAN_EXTRA_ARGS=""
if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ -f /etc/os-release ]]; then
    CONAN_EXTRA_ARGS="-c tools.system.package_manager:mode=install"
fi

if [ "$TARGET" == "server" ]; then
    conan install ./conanfile_server.txt --output-folder=.build --build=missing --profile:build=default --profile:host=default --settings "build_type=$BUILD_TYPE"
else
    conan install . --output-folder=.build --build=missing --profile:build=default --profile:host=default --settings "build_type=$BUILD_TYPE"
fi

case $TARGET in
    "client")
        echo "Building client only..."
        cmake -B .build -DCMAKE_TOOLCHAIN_FILE=".build/conan_toolchain.cmake" -DBUILD_CLIENT=ON -DBUILD_SERVER=OFF -DCMAKE_BUILD_TYPE=$BUILD_TYPE
        cmake --build .build --config $BUILD_TYPE
        ;;
    "server")
        echo "Building server only..."
        cmake -B .build -DCMAKE_TOOLCHAIN_FILE=".build/conan_toolchain.cmake" -DBUILD_CLIENT=OFF -DBUILD_SERVER=ON -DCMAKE_BUILD_TYPE=$BUILD_TYPE
        cmake --build .build --config $BUILD_TYPE
        ;;
    "both")
        echo "Building both client and server..."
        cmake -B .build -DCMAKE_TOOLCHAIN_FILE=".build/conan_toolchain.cmake" -DBUILD_CLIENT=ON -DBUILD_SERVER=ON -DCMAKE_BUILD_TYPE=$BUILD_TYPE
        cmake --build .build --config $BUILD_TYPE
        ;;
    *)
        echo "Invalid target. Use: client, server, or both"
        exit 1
        ;;
esac

echo "Build completed successfully!"
echo "Executables are located at the root directory"
