find_package(asio REQUIRED)

set(PROJECT_NAME r_type_server)

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

file(GLOB_RECURSE GAME_ENGINE_SOURCES CONFIGURE_DEPENDS "../game_engine/ecs/*.cpp" "../game_engine/ecs/components/*.cpp" "../game_engine/ecs/systems/*.cpp")

add_executable(${PROJECT_NAME}
  ${SERVER_SOURCES}
  ${GAME_ENGINE_SOURCES}
)

add_compile_options(-W -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} asio::asio)

target_include_directories(${PROJECT_NAME} PRIVATE
  src/
  src/errors/
  src/packets/
  src/game/
  src/player/
  ../core/network/
  ../game_engine/ecs/
  ../game_engine/ecs/components/
  ../game_engine/ecs/systems/
)
