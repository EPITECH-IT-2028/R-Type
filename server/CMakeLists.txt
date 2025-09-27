find_package(asio REQUIRED)
find_package(Threads REQUIRED)

set(PROJECT_NAME r_type_server)

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE GAME_ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/game_engine/ecs/*.cpp")
list(REMOVE_ITEM GAME_ENGINE_SOURCES "${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/InputSystem.cpp")

add_executable(${PROJECT_NAME}
  ${SERVER_SOURCES}
  ${GAME_ENGINE_SOURCES}
)
add_compile_options(-W -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} asio::asio Threads::Threads)
target_include_directories(${PROJECT_NAME} PRIVATE
  src/
  src/errors/
  src/packets/
  src/game/
  src/player/
  src/projectile/
  ${CMAKE_SOURCE_DIR}/core/network/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/components/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/
)
