cmake_minimum_required(VERSION 3.28)
project(gameboy)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wformat -O0 -g -fno-access-control -fno-exceptions -fno-rtti -fno-omit-frame-pointer -fsanitize=address")

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenGL REQUIRED)

set(SRC_DIR ${ROOT_DIR}/src)
set(THIRD_PARTY_DIR ${SRC_DIR}/3rdparty)
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)

if (EMSCRIPTEN)
    list(APPEND LINK_FLAGS_LIST
            "-s USE_PTHREADS=1"
            "-s PTHREAD_POOL_SIZE=1"
            "-s FORCE_FILESYSTEM=1"
            "-s EXPORTED_RUNTIME_METHODS=ccall"
            "-s EXPORTED_FUNCTIONS=['_main','_LoadRom']"
            "-s USE_GLFW=3"
            "-s USE_WEBGL2=1"
            "-s FULL_ES3=1"
            "-s WASM=1"
            "-s ALLOW_MEMORY_GROWTH=1"
            "-s NO_EXIT_RUNTIME=0"
            "-s ASSERTIONS=1"
            "--shell-file ${SRC_DIR}/wasm.html"
            "--shared-memory"
    )
    string(JOIN " " LINK_FLAGS ${LINK_FLAGS_LIST})

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wformat -O2 -msimd128 -matomics -mbulk-memory")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS})
    set(CMAKE_LINK_LIBRARY_FLAG ${CMAKE_LINK_LIBRARY_FLAG} ${LINK_FLAGS})
endif ()


include_directories(${THIRD_PARTY_DIR}/nameof/include)
include_directories(${THIRD_PARTY_DIR}/imgui)
include_directories(${THIRD_PARTY_DIR}/imgui/backends)
include_directories(${THIRD_PARTY_DIR}/miniaudio)
include_directories(${SRC_DIR})

add_subdirectory(${THIRD_PARTY_DIR}/nameof)
add_subdirectory(${THIRD_PARTY_DIR}/googletest)

set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

file(GLOB_RECURSE GB_HEADERS
        ${SRC_DIR}/cartridge/*.h
        ${SRC_DIR}/common/*.h
        ${SRC_DIR}/machine/*.h
        ${SRC_DIR}/debugger/*.h
)


file(GLOB_RECURSE GB_SRCS
        ${SRC_DIR}/cartridge/*.cpp
        ${SRC_DIR}/common/*.cpp
        ${SRC_DIR}/machine/*.cpp
)

add_executable(gameboy
        src/main.cpp
        ${GB_SRCS}
        ${IMGUI_SOURCES}
)


if (NOT EMSCRIPTEN)
    add_executable(gb_test
            ${SRC_DIR}/test/run_tests.cpp
            ${SRC_DIR}/test/test_set.cpp
            ${GB_SRCS}
    )
    target_compile_options(gb_test PRIVATE -O3)
    target_link_libraries(gameboy glfw OpenGL::GL m pthread dl)
    target_link_libraries(gb_test GTest::gtest_main)
endif ()
