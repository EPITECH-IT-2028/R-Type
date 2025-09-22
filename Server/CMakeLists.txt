find_package(asio REQUIRED)

set(PROJECT_NAME r_type_server)

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME}
  ${SERVER_SOURCES}
)
add_compile_options(-W -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} asio::asio)
target_include_directories(${PROJECT_NAME} PRIVATE
  src/
  src/errors/
  src/packets/
  src/game/
  ../core/network/
)
