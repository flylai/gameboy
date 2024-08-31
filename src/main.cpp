#include <cstdio>

#include "common/defs.h"
#include "common/type.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "include/gameboy_c.h"
#include "machine/gameboy.h"
#include "widget/main_ui.h"
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

static std::function<void()> RenderLoop;

GameBoy g_gameboy{};
gb::Widgets g_imgui_widgets;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static void MainLoopForEmscripten() { RenderLoop(); }
#endif


extern "C" void LoadRom(const char* path) {
  if (g_gameboy) {
    GameBoyDestroy(g_gameboy);
  }
  g_gameboy = GameBoyInit(path);
  g_imgui_widgets.setGameBoy((gb::GameBoy*) g_gameboy);
  GameBoyRunWithNewThread(g_gameboy);
}

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int argc, char* argv[]) {
#ifndef __EMSCRIPTEN__
  if (argc < 2) {
    GB_LOG(WARN) << "Usage: " << argv[0] << " <rom>";
    return -1;
  }
#endif

  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) return 1;

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char* glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
  if (window == nullptr) return 1;
  glfwMakeContextCurrent(window);

#ifdef __EMSCRIPTEN__
  glfwSwapInterval(0); // Disable vsync
#else
  glfwSwapInterval(1); // Enable vsync
#endif

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void) io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
  ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Our state
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  if (argc > 1) {
    LoadRom(argv[1]);
  }

  RenderLoop = [&]() {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    g_imgui_widgets.draw();

    //  Joypad status
    {
#define DEF_KEY(KEY, ACTION)                           \
  if (ImGui::IsKeyPressed(ImGuiKey_##KEY)) {           \
    ((gb::GameBoy*) g_gameboy)->joypad_.ACTION(true);  \
  }                                                    \
  if (ImGui::IsKeyReleased(ImGuiKey_##KEY)) {          \
    ((gb::GameBoy*) g_gameboy)->joypad_.ACTION(false); \
  }

      DEF_KEY(A, Left)
      DEF_KEY(W, Up)
      DEF_KEY(D, Right)
      DEF_KEY(S, Down)

      DEF_KEY(J, A)
      DEF_KEY(K, B)
      DEF_KEY(Enter, Start)
      DEF_KEY(Space, Select)
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                 clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  };

  // Main loop
#ifdef __EMSCRIPTEN__
  // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
  // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.

  io.IniFilename = nullptr;
  emscripten_set_main_loop(MainLoopForEmscripten, 0, true);
#else
  while (!glfwWindowShouldClose(window)) {
    RenderLoop();
  }
#endif // __EMSCRIPTEN__

  // Cleanup
  GameBoyDestroy(g_gameboy);

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
