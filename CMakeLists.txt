cmake_minimum_required(VERSION 3.27.4)
project(r_type)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
execute_process(
  COMMAND ln -sf ${PROJECT_SOURCE_DIR}/.build/compile_commands.json ${PROJECT_SOURCE_DIR}
)

option(BUILD_CLIENT "Build the client" OFF)
option(BUILD_SERVER "Build the server" OFF)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(WARNING "CMAKE_TOOLCHAIN_FILE is not set. Please set it via -DCMAKE_TOOLCHAIN_FILE=...")
endif()

if(NOT BUILD_CLIENT AND NOT BUILD_SERVER)
  message(FATAL_ERROR "At least one of BUILD_CLIENT or BUILD_SERVER must be ON.")
endif()

if(BUILD_CLIENT)
  add_subdirectory(client)
endif()

if(BUILD_SERVER)
  add_subdirectory(Server)
endif()
