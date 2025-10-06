find_package(raylib REQUIRED)

set(PROJECT_NAME r_type_client)

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS "*.cpp")
set(ECS_SOURCES
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/EntityManager.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/RenderSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/InputSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/MovementSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/BackgroundSystem.cpp
)

add_executable(${PROJECT_NAME}
  ${CLIENT_SOURCES}
  ${ECS_SOURCES}
)
add_compile_options(-W -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} raylib)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/client
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/components/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/tags/
)
