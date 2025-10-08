find_package(asio REQUIRED)
find_package(Threads REQUIRED)

set(PROJECT_NAME r_type_server)

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE ALL_GAME_ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/game_engine/ecs/*.cpp")

set(GAME_ENGINE_SOURCES "")
foreach(source_file ${ALL_GAME_ENGINE_SOURCES})
    execute_process(
        COMMAND grep -l "raylib\.h" ${source_file}
        RESULT_VARIABLE result
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT result EQUAL 0)
        list(APPEND GAME_ENGINE_SOURCES ${source_file})
    endif()
endforeach()

set(CORE_SOURCES
  ${CMAKE_SOURCE_DIR}/core/Parser.cpp
)

add_executable(${PROJECT_NAME}
  ${SERVER_SOURCES}
  ${GAME_ENGINE_SOURCES}
  ${CORE_SOURCES}
)

add_compile_options(-W -Wall -Wextra)
target_link_libraries(${PROJECT_NAME} asio::asio Threads::Threads)
target_include_directories(${PROJECT_NAME} PRIVATE
  src/
  src/errors/
  src/packets/
  src/game/
  src/player/
  src/enemy/
  src/queue/
  src/projectile/
  ${CMAKE_SOURCE_DIR}/core/
  ${CMAKE_SOURCE_DIR}/core/errors/
  ${CMAKE_SOURCE_DIR}/core/network/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/components/
  ${CMAKE_SOURCE_DIR}/game_engine/ecs/systems/
)
