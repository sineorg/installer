cmake_minimum_required(VERSION 3.16)

# Use vcpkg toolchain if available
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
endif()

project(sine_installer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force static linking for your code and vcpkg libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Windows-specific: static runtime
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Define static linking for GLFW before including
if(WIN32)
    add_compile_definitions(GLFW_STATIC)
endif()

# Add executable
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

# Windows-specific: GUI app (no console)
if(WIN32)
    set_target_properties(sine_installer PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Include vcpkg-installed packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(CURL REQUIRED)
find_package(minizip-ng REQUIRED)

# Include directories
target_include_directories(sine_installer PRIVATE
    external/glad/include
    external/imgui
    external/imgui/backends
    external/fonts
)

# MSVC-specific legacy stdio definitions
if(MSVC)
    target_link_libraries(sine_installer PRIVATE legacy_stdio_definitions)
endif()

# Link libraries
target_link_libraries(sine_installer PRIVATE
    glfw
    OpenGL::GL
    CURL::libcurl
    MINIZIP::minizip-ng
)

# Include source directory
include_directories(src)
