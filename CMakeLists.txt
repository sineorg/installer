cmake_minimum_required(VERSION 3.16)

# Use vcpkg toolchain if available
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
endif()

project(sine_installer)
set(CMAKE_CXX_STANDARD 17)

# Force static CRT on Windows
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # /MT or /MTd
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

# Windows-specific properties
if(WIN32)
    add_definitions(-DGLFW_STATIC)  # Tell GLFW we're linking statically
    set_target_properties(sine_installer PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Dependencies
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

# Link libraries (static versions)
if(MSVC)
    # Needed for old symbols in some static libs
    target_link_libraries(sine_installer PRIVATE legacy_stdio_definitions)
endif()

target_link_libraries(sine_installer PRIVATE
    glfw
    OpenGL::GL
    CURL::libcurl
    MINIZIP::minizip-ng
)

# Include src folder for convenience
include_directories(src)


include_directories(src)
