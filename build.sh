#!/bin/bash

# Usage: ./build.sh [debug|release] (defaults to both)
#        ./build.sh [client|server|both] [debug|release]

set -e

if [ $# -eq 0 ]; then
    TARGET="both"
    BUILD_TYPE="Release"
elif [ $# -eq 1 ]; then
    case $1 in
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
            echo "Invalid argument. Use: client, server, both, debug, or release"
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

# Clean and recreate build directory to avoid any cache issues
if [ -d ".build" ]; then
    echo "Cleaning existing build directory..."
    rm -rf .build
fi

# Install conan dependencies
echo "Installing conan dependencies..."
conan install . --output-folder=.build --build=missing --profile:build=default --profile:host=default

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
echo "Executables are in .build/bin/"
