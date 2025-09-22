find_package(asio REQUIRED)

add_compile_options(-W -Wall -Wextra)
add_executable(${PROJECT_NAME}
  src/main.cpp
  src/Server.cpp
  Packets/Packet.cpp
)
target_link_libraries(${PROJECT_NAME} asio::asio)
target_include_directories(${PROJECT_NAME} PRIVATE src)
