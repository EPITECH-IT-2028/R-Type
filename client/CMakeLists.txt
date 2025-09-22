find_package(raylib REQUIRED)

set(PROJECT_NAME r_type_client)
add_executable(${PROJECT_NAME}
  main.cpp
)
add_compile_options(-W -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} raylib)
