cmake_minimum_required(VERSION 3.0)

project(file2hex4c C)

set(CMAKE_C_COMPILE_FEATURES "${CMAKE_C_FLAGS} -Wunused-result -Wl,-q -Wall -O3 -fno-inline -fno-builtin")

add_executable(file2hex4c
  src/main.c
)

target_link_libraries(file2hex4c
)

install(TARGETS file2hex4c DESTINATION bin)
