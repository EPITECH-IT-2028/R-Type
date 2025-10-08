#!/bin/bash

# Usage: ./build.sh [debug|release] (defaults to both)
#        ./build.sh [client|server|both] [debug|release]
#        ./build.sh clean

set -e

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
if ! conan profile show default > /dev/null 2>&1; then
    echo "[INFO] Conan default profile not found. Detecting..."
    conan profile detect --force > /dev/null 2>&1
fi

CONAN_EXTRA_ARGS=""
if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ -f /etc/os-release ]]; then
    CONAN_EXTRA_ARGS="-c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True"
fi

conan install . --output-folder=.build --build=missing --profile:build=default --profile:host=default --settings "build_type=$BUILD_TYPE" $CONAN_EXTRA_ARGS

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
