find_package(raylib REQUIRED)
find_package(asio REQUIRED)

set(PROJECT_NAME r_type_client)

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS "*.cpp")
set(ECS_SOURCES
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/EntityManager.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/RenderSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/InputSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/MovementSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/BackgroundSystem.cpp
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/BoundarySystem.cpp
)

add_executable(${PROJECT_NAME}
  ${CLIENT_SOURCES}
  ${ECS_SOURCES}
)

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE 
    _WIN32_WINNT=0x0601
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    NOGDI
  )
  target_compile_options(${PROJECT_NAME} PRIVATE -include ${CMAKE_CURRENT_SOURCE_DIR}/WindowsFix.hpp)
endif()

add_compile_options(-W -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} raylib asio::asio)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/client/
  ${CMAKE_SOURCE_DIR}/client/packets/
  ${CMAKE_SOURCE_DIR}/core/network/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/components/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/tags/
)
