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
    GIT_TAG 231cb29bab30f82712fcd54faaea42424cc6e710
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(asio)

add_executable(r_type_server
    src/main.cpp
    src/Server.cpp
    src/Packets/Packet.cpp
    src/Help.cpp
    src/Parser.cpp
)

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

