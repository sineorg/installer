cmake_minimum_required(VERSION 3.16)

# Use vcpkg toolchain if available
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
endif()

project(sine_installer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force static linking where possible
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

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

# Windows-specific settings
if(WIN32)
    # Build a Windows GUI executable (no console)
    set_target_properties(sine_installer PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    # Static runtime to avoid MSVCRT DLL dependency
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # Tell GLFW we are using static linking
    add_definitions(-DGLFW_INCLUDE_NONE)
    add_definitions(-DGLFW_STATIC)
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

# MSVC-specific libraries
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
