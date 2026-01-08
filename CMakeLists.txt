cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if (WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static")
endif()

project(sine_installer LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(WIN32)
    add_compile_definitions(GLFW_STATIC)
endif()

add_executable(sine_installer
    src/main.cpp
    src/data.cpp
    external/glad/src/gl.c
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

if(WIN32)
    set_target_properties(sine_installer PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
endif()

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(CURL REQUIRED)
find_package(minizip-ng REQUIRED)

target_include_directories(sine_installer PRIVATE
    external/glad/include
    external/imgui
    external/imgui/backends
    external/fonts
)

if(MSVC)
    target_link_libraries(sine_installer PRIVATE legacy_stdio_definitions)
endif()

target_link_libraries(sine_installer PRIVATE
    glfw
    OpenGL::GL
    CURL::libcurl
    MINIZIP::minizip-ng
)

include_directories(src)
