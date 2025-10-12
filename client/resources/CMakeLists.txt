set(EMBEDDED_ASSETS_DIR "${CMAKE_BINARY_DIR}/embedded_assets")
file(MAKE_DIRECTORY ${EMBEDDED_ASSETS_DIR})

add_executable(embed_assets
  ../EmbedAssets.cpp
  ../AssetManager.cpp
)
target_link_libraries(embed_assets raylib)
target_include_directories(embed_assets PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(REQUIRED_ASSETS background.png players.gif enemies.gif)
foreach(asset ${REQUIRED_ASSETS})
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${asset}")
    message(FATAL_ERROR "Required asset not found: ${asset}")
  endif()
endforeach()


add_custom_command(
  OUTPUT
    ${EMBEDDED_ASSETS_DIR}/background_data.h
    ${EMBEDDED_ASSETS_DIR}/players_data.h
    ${EMBEDDED_ASSETS_DIR}/enemies_data.h
  COMMAND embed_assets
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EMBEDDED_ASSETS_DIR}
  DEPENDS
    embed_assets
    ${CMAKE_CURRENT_SOURCE_DIR}/background.png
    ${CMAKE_CURRENT_SOURCE_DIR}/players.gif
    ${CMAKE_CURRENT_SOURCE_DIR}/enemies.gif
  COMMENT "Generating embedded asset headers..."
  VERBATIM
)

add_custom_target(generate_embedded_assets ALL
  COMMAND ${CMAKE_COMMAND} -E remove -f "${PROJECT_SOURCE_DIR}/embed_assets"
  DEPENDS
    ${EMBEDDED_ASSETS_DIR}/background_data.h
    ${EMBEDDED_ASSETS_DIR}/players_data.h
    ${EMBEDDED_ASSETS_DIR}/enemies_data.h
)
