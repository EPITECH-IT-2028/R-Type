find_package(raylib REQUIRED)

add_compile_options(-W -Wall -Wextra)
add_executable(${PROJECT_NAME}
  main.cpp
)
target_link_libraries(${PROJECT_NAME} raylib)
