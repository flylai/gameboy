#include <cstdio>

#include "common/type.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "include/gameboy_c.h"
#include "machine/gameboy.h"
#include "nameof.hpp"
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

static std::function<void()> RenderLoop;
GameBoy g_gameboy{};


#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static void MainLoopForEmscripten() { RenderLoop(); }
#endif


extern "C" void LoadRom(const char* path) {
  if (g_gameboy) {
    GameBoyDestroy(g_gameboy);
  }
  g_gameboy = GameBoyInit(path);
  GameBoyRunWithNewThread(g_gameboy);
}

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

inline void decode_tile(gb::u16 data, gb::u8* target) {
  gb::u8 lo                    = data & 0xff;
  gb::u8 hi                    = data >> 8;
  constexpr gb::u8 color_map[] = {
          8,   24,  32,  //
          52,  104, 86,  //
          73,  126, 50,  //
          224, 248, 208, //
  };
  for (gb::u8 i = 7; i > 0; i--) {
    gb::u8 color1 = getBitN(lo, i);
    gb::u8 color2 = getBitN(hi, i);
    gb::u8 color  = (color2 << 1) | color1;
    GB_ASSERT(color <= 3);
    target[(7 - i) * 3 + 0] = color_map[color * 3 + 0];
    target[(7 - i) * 3 + 1] = color_map[color * 3 + 1];
    target[(7 - i) * 3 + 2] = color_map[color * 3 + 2];
  }
}

bool updateTileMap(GameBoy gb, GLuint* out_texture, int* out_width, int* out_height) {
#ifdef __EMSCRIPTEN__
  // WASM will throw exception about "RuntimeError: memory access out of bounds"
  return false;
#else
  if (!gb) {
    return false;
  }
  auto* gameboy                     = (gb::GameBoy*) gb;
  constexpr gb::u16 VRAM_BASE       = 0x8000;
  constexpr gb::u16 MAX_TILE_COUNT  = (0x97ff - 0x8000 + 1) / 16;
  constexpr gb::u16 TILE_PER_ROW    = 16;
  constexpr gb::u16 TILE_PER_COL    = MAX_TILE_COUNT / 16;
  constexpr gb::u16 WIDTH           = TILE_PER_ROW * 8;
  constexpr gb::u16 HEIGHT          = TILE_PER_COL * 8;
  gb::u8 pixels[WIDTH * HEIGHT * 3] = {0};
  gb::u16 tile_idx{};

  for (gb::u16 addr = VRAM_BASE; addr < 0x97ff; addr += 16) {
    for (gb::u8 i = 0; i < 8; i++) {
      // 3 means RGB,
      gb::u32 offset = (tile_idx % 16) * 8 * 3              // x offset
                       + ((tile_idx / 16) * 8) * 16 * 8 * 3 // y offset
              ;
      gb::u16 hi = gameboy->memory_bus_.get(VRAM_BASE + tile_idx * 16 + i * 2);
      gb::u16 lo = gameboy->memory_bus_.get(VRAM_BASE + tile_idx * 16 + i * 2 + 1);
      decode_tile(hi << 8 | lo, pixels + offset + i * 16 * 8 * 3);
    }
    tile_idx++;
  }

  static bool init = false;
  static GLuint image_texture;
  if (!init) {
    init = true;
    glGenTextures(1, &image_texture);
  }
  glBindTexture(GL_TEXTURE_2D, image_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  *out_texture = image_texture;
  *out_width   = WIDTH;
  *out_height  = HEIGHT;
#endif
  return true;
}

bool updateGameboyLCD(GameBoy gb, GLuint* out_texture, int* out_width, int* out_height, gb::u8 scale) {
  if (!gb) {
    return false;
  }

  constexpr gb::u32 WIDTH     = 160;
  constexpr gb::u32 HEIGHT    = 144;
  const gb::u32 SCALED_WIDTH  = WIDTH * scale;
  const gb::u32 SCALED_HEIGHT = HEIGHT * scale;

  static bool init            = false;
  static GLuint image_texture;
  if (!init) {
    init = true;
    glGenTextures(1, &image_texture);
  }
  glBindTexture(GL_TEXTURE_2D, image_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  const gb::u8* original_data = (gb::u8*) GameBoyTextureBuffer(gb);
  std::vector<gb::u8> scaled_data(SCALED_WIDTH * SCALED_HEIGHT * 4);

  for (gb::u32 y = 0; y < HEIGHT; ++y) {
    for (gb::u32 x = 0; x < WIDTH; ++x) {
      gb::u32 src_index = (y * WIDTH + x) * 4;
      for (gb::u32 dy = 0; dy < scale; ++dy) {
        for (gb::u32 dx = 0; dx < scale; ++dx) {
          gb::u32 dst_index = ((y * scale + dy) * SCALED_WIDTH + (x * scale + dx)) * 4;
          std::copy(original_data + src_index, original_data + src_index + 4, scaled_data.data() + dst_index);
        }
      }
    }
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCALED_WIDTH, SCALED_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               scaled_data.data());

  *out_texture = image_texture;
  *out_width   = SCALED_WIDTH;
  *out_height  = SCALED_HEIGHT;
  return true;
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

    {
      int tile_texture_width  = 0;
      int tile_texture_height = 0;
      GLuint tile_texture_id  = 0;
      ImGui::Begin("Tile Map");
      updateTileMap(g_gameboy, &tile_texture_id, &tile_texture_width, &tile_texture_height);
      ImGui::Text("pointer = %x", tile_texture_id);
      ImGui::Text("size = %d x %d", tile_texture_width, tile_texture_height);
      ImGui::Image((void*) (intptr_t) tile_texture_id, ImVec2(tile_texture_width, tile_texture_height));
      ImGui::End();
    }

    {
      int game_texture_width  = 0;
      int game_texture_height = 0;
      static int scale        = 3;
      GLuint game_texture_id  = 0;
      ImGui::Begin("Game");
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
      updateGameboyLCD(g_gameboy, &game_texture_id, &game_texture_width, &game_texture_height, scale);
      ImGui::Text("pointer = %x", game_texture_id);
      ImGui::Text("size = %d x %d", game_texture_width, game_texture_height);
      ImGui::SliderInt("Scale", &scale, 1, 16);
      ImGui::Image((void*) (intptr_t) game_texture_id, ImVec2(game_texture_width, game_texture_height));
      ImGui::End();
    }
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
