find_package(raylib REQUIRED)

set(PROJECT_NAME r_type_client)

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(${PROJECT_NAME}
  ${SERVER_SOURCES}
)
add_compile_options(-W -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} raylib)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/components/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/
)
