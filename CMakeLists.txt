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
option(BUILD_TESTS "Build the tests" OFF)

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
  add_subdirectory(server)
endif()

if(BUILD_TESTS)
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
