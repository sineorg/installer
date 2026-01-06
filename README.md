# GLFW + OpenGL + ImGui (Embedded Font) — Full Working Setup

This is a **clean, minimal, correct** setup that matches everything we discussed:

* GLFW + modern OpenGL
* GLAD (loader-based, `gladLoadGL(glfwGetProcAddress)`)
* ImGui
* Full-screen overlay layout
* Bottom-left / bottom-right buttons
* Disabled buttons
* Embedded **monospace font** (Cascadia Code) — **no runtime font files**

> ⚠️ Font binary headers are **generated**, not handwritten. Instructions are included.

---

## 📁 Project Structure

```
cpp-installer/
├── CMakeLists.txt
├── src/
│   └── main.cpp
├── external/
│   ├── glad/
│   │   ├── include/
│   │   │   └── glad/gl.h
│   │   └── src/gl.c
│   ├── glfw/              (or system-installed glfw)
│   ├── imgui/
│   │   ├── imgui.cpp
│   │   ├── imgui_draw.cpp
│   │   ├── imgui_tables.cpp
│   │   ├── imgui_widgets.cpp
│   │   ├── imgui.h
│   │   └── backends/
│   │       ├── imgui_impl_glfw.cpp
│   │       ├── imgui_impl_glfw.h
│   │       ├── imgui_impl_opengl3.cpp
│   │       └── imgui_impl_opengl3.h
│   └── fonts/
│       ├── CascadiaCode-Regular.ttf
│       ├── CascadiaCode-Bold.ttf
│       ├── CascadiaCode-Regular.h   (generated)
│       └── CascadiaCode-Bold.h      (generated)
```

---

## 🧠 Generate Embedded Font Headers (REQUIRED)

Run this **once** from `external/fonts/`:

```bash
xxd -i CascadiaCode-Regular.ttf > CascadiaCode-Regular.h
xxd -i CascadiaCode-Bold.ttf > CascadiaCode-Bold.h
```

These `.h` files contain byte arrays and are compiled directly into your binary.

---

## 🧩 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(glfw_opengl)

set(CMAKE_CXX_STANDARD 17)

add_executable(glfw_opengl
    src/main.cpp
    external/glad/src/gl.c
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)


target_include_directories(glfw_opengl PRIVATE
    external/glad/include
    external/imgui
    external/imgui/backends
    external/fonts
)


target_link_libraries(glfw_opengl PRIVATE
    glfw
    OpenGL::GL
)
```

---

## 🚀 src/main.cpp (FULL FILE)

```cpp
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "CascadiaCode-Regular.h"
#include "CascadiaCode-Bold.h"

#include <iostream>

static void framebuffer_size_callback(GLFWwindow*, int w, int h)
{
    glViewport(0, 0, w, h);
}

int main()
{
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(900, 600, "iminstaller", nullptr, nullptr);
    if (!window) return -1;

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGL(glfwGetProcAddress)) return -1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // === Embedded fonts ===
    ImGuiIO& io = ImGui::GetIO();

    ImFont* bodyFont = io.Fonts->AddFontFromMemoryTTF(
        CascadiaCode_Regular_ttf,
        CascadiaCode_Regular_ttf_len,
        18.0f
    );

    ImFont* titleFont = io.Fonts->AddFontFromMemoryTTF(
        CascadiaCode_Bold_ttf,
        CascadiaCode_Bold_ttf_len,
        36.0f
    );

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImVec2 size = ImGui::GetIO().DisplaySize;

        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(size);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground;

        ImGui::Begin("Overlay", nullptr, flags);

        ImGui::PushFont(titleFont);
        ImGui::Text("iminstaller");
        ImGui::PopFont();
        ImGui::Separator();

        ImGui::PushFont(bodyFont);
        ImGui::Text("Welcome. Choose an action below.");
        ImGui::PopFont();

        // Bottom-left button
        float bw = 120, bh = 28;
        ImGui::SetCursorPos(ImVec2(10, size.y - bh - 10));
        ImGui::BeginDisabled(true);
        ImGui::Button("Back", ImVec2(bw, bh));
        ImGui::EndDisabled();

        // Bottom-right button
        ImGui::SetCursorPos(ImVec2(size.x - bw - 10, size.y - bh - 10));
        if (ImGui::Button("Next", ImVec2(bw, bh)))
            glfwSetWindowShouldClose(window, true);

        ImGui::End();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}
```

---

## ✅ What You Have Now

* ✔ Embedded monospace font
* ✔ Full-screen UI overlay
* ✔ Big title + content
* ✔ Bottom-left & bottom-right buttons
* ✔ Disabled buttons
* ✔ Clean, scalable layout

This is **exactly how real ImGui tools and launchers are built**.

---

## ➡️ Natural Next Steps

* Multi-page UI state machine
* Progress bars / install steps
* Theme customization
* Keyboard navigation
* DPI scaling

When you’re ready, tell me what direction you want to go next and we’ll build it cleanly on top of this.

