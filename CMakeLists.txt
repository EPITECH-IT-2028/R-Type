cmake_minimum_required(VERSION 3.16)
project(RType)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(
  COMMAND ln -sf ${PROJECT_SOURCE_DIR}/.build/compile_commands.json ${PROJECT_SOURCE_DIR}
)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
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

option(SERVER "Build the server executable" ON)
option(GAME "Build the game executable" ON)

if(SERVER)
    add_executable(r_type_server
        server/src/main.cpp
        server/src/Server.cpp
        server/src/Packets/PacketFactory.cpp
        server/src/Packets/PacketHandler.cpp
    )

    set_target_properties(r_type_server PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

    target_link_libraries(r_type_server 
        PRIVATE 
        Threads::Threads
    )

    target_include_directories(r_type_server 
        PRIVATE 
        ${asio_SOURCE_DIR}/asio/include
        server/src/
        server/src/Packets/
        core/network/
    )

    target_compile_definitions(r_type_server PRIVATE ASIO_STANDALONE)
endif()

if(GAME)
endif()
