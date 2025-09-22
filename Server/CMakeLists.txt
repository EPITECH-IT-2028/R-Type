find_package(asio REQUIRED)

set(PROJECT_NAME r_type_server)
add_executable(r_type_server
  src/main.cpp
  src/Server.cpp
  src/Packets/Packet.cpp
)
add_compile_options(-W -Wall -Wextra)

target_link_libraries(r_type_server asio::asio)
target_include_directories(r_type_server PRIVATE src src/Packets)
