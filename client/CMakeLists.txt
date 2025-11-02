find_package(raylib REQUIRED)
find_package(asio REQUIRED)
find_package(Threads REQUIRED)
find_package(Bitsery REQUIRED CONFIG)
find_package(PicoSHA2 REQUIRED CONFIG)
find_package(lz4 REQUIRED)

set(PROJECT_NAME r_type_client)

add_subdirectory(resources)

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS "*.cpp")
list(FILTER CLIENT_SOURCES EXCLUDE REGEX "EmbedAssets.cpp")

file(GLOB_RECURSE CORE_NETWORK_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/core/network/ClientNetworkManager.cpp")
set(ECS_SOURCES
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/EntityManager.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/RenderSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/InputSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/MovementSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/BackgroundSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/SpriteAnimationSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/ProjectileSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/MetricsSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/InterpolationSystem.cpp
)

set(CORE_SOURCES
  ${CMAKE_SOURCE_DIR}/core/Parser.cpp
  ${CMAKE_SOURCE_DIR}/core/network/ClientNetworkManager.cpp
)

add_executable(${PROJECT_NAME}
  ${CLIENT_SOURCES}
  ${ECS_SOURCES}
  ${CORE_SOURCES}
  ${CORE_NETWORK_SOURCES}
)
if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX _WIN32_WINNT=0x0A00 NOGDI NOUSER)
endif()
add_dependencies(${PROJECT_NAME} generate_embedded_assets)

add_compile_options(-W -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} raylib asio::asio Threads::Threads Bitsery::bitsery picosha2::picosha2 lz4::lz4)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/client/
  ${CMAKE_SOURCE_DIR}/client/packets/
  ${CMAKE_SOURCE_DIR}/core/
  ${CMAKE_SOURCE_DIR}/core/errors/
  ${CMAKE_SOURCE_DIR}/core/network/
  ${CMAKE_SOURCE_DIR}/core/utils/
  ${CMAKE_SOURCE_DIR}/core/crypto/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/components/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/tags/
  ${CMAKE_BINARY_DIR}/embedded_assets
)

target_compile_definitions(${PROJECT_NAME} PRIVATE ASIO_STANDALONE)
