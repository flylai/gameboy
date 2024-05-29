cmake_minimum_required(VERSION 3.28)
project(gameboy)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -fno-omit-frame-pointer -fsanitize=address")

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)

set(SRC_DIR ${ROOT_DIR}/src)
include_directories(${SRC_DIR}/3rdparty)
include_directories(${SRC_DIR})

include_directories(${SRC_DIR}/3rdparty/glad/include)

file(GLOB_RECURSE GB_HEADERS ${SRC_DIR}/*.h)

add_executable(gameboy src/main.cpp ${GB_HEADERS} ${SRC_DIR}/machine/cpu.cpp)


target_link_libraries(gameboy glfw OpenGL::GL)
