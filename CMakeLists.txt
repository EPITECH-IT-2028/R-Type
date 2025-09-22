cmake_minimum_required(VERSION 3.16)
project(RType)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${PROJECT_SOURCE_DIR}/compile_commands.json
  )
endif()

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
    GIT_TAG asio-1-36-0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(asio)

option(SERVER "Build the server executable" OFF)
option(GAME "Build the game executable" OFF)
option(ENABLE_TESTS "Build the tests" OFF)

if(SERVER)
    add_executable(r_type_server
        server/src/main.cpp
        server/src/Server.cpp
        server/src/packets/PacketFactory.cpp
        server/src/packets/PacketHandler.cpp
        server/src/Help.cpp
        server/src/Parser.cpp
        server/src/game/Game.cpp
        server/src/player/Player.cpp
        game_engine/ecs/EntityManager.cpp
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
        server/src/errors/
        server/src/packets/
        server/src/game/
        server/src/player/
        core/network/
        game_engine/ecs/components/
        game_engine/ecs/systems/
        game_engine/ecs/
    )

    target_compile_definitions(r_type_server PRIVATE ASIO_STANDALONE)
endif()

if(ENABLE_TESTS)
    enable_testing()

    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.17.0
      GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(unit_tests
      server/tests/test_server.cpp
      server/src/Server.cpp
    )

    target_include_directories(unit_tests PRIVATE
      ${asio_SOURCE_DIR}/asio/include
      ${CMAKE_CURRENT_SOURCE_DIR}/server/src
      ${CMAKE_CURRENT_SOURCE_DIR}/server/src/packets/
      ${CMAKE_CURRENT_SOURCE_DIR}/server/src/errors/
      ${CMAKE_CURRENT_SOURCE_DIR}/core/network/
    )

    target_link_libraries(unit_tests PRIVATE
      GTest::gtest_main
      Threads::Threads
    )

    target_compile_definitions(unit_tests PRIVATE ASIO_STANDALONE)

    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()

if(GAME)
endif()
