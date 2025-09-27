find_package(raylib REQUIRED)

set(PROJECT_NAME r_type_client)

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

file(GLOB_RECURSE GAME_ENGINE_SOURCES CONFIGURE_DEPENDS "../game_engine/ecs/*.cpp")

add_executable(${PROJECT_NAME}
  ${CLIENT_SOURCES}
  ${GAME_ENGINE_SOURCES}
)
add_compile_options(-W -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} raylib)

target_include_directories(${PROJECT_NAME} PRIVATE
  client/
  ../core/network/
  ../game_engine/ecs/
  ../game_engine/ecs/components/
  ../game_engine/ecs/systems/
)