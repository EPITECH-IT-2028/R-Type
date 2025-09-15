cmake_minimum_required(VERSION 3.16)
project(RTypeServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(
  COMMAND ln -sf ${PROJECT_SOURCE_DIR}/.build/compile_commands.json ${PROJECT_SOURCE_DIR}
)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-36-0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(asio)

file(GLOB_RECURSE SERVER_SOURCES "src/*.cpp" "src/*.hpp")

add_executable(r_type_server ${SERVER_SOURCES})

target_link_libraries(r_type_server 
    PRIVATE 
    Threads::Threads
)

target_include_directories(r_type_server 
    PRIVATE 
    ${asio_SOURCE_DIR}/asio/include
    src/
    src/Packets/
)

target_compile_definitions(r_type_server PRIVATE ASIO_STANDALONE)

