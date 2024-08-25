#include <cstdio>

#include "common/type.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

#include <functional>
static std::function<void()> MainLoopForEmscriptenP;

static void MainLoopForEmscripten() { MainLoopForEmscriptenP(); }

#define EMSCRIPTEN_MAINLOOP_BEGIN MainLoopForEmscriptenP = [&]()
#define EMSCRIPTEN_MAINLOOP_END \
  ;                             \
  emscripten_set_main_loop(MainLoopForEmscripten, 0, true)
#else
#define EMSCRIPTEN_MAINLOOP_BEGIN
#define EMSCRIPTEN_MAINLOOP_END
#endif


#include <cmath>

#include "machine/gameboy.h"
#include "nameof.hpp"

using namespace gb;

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

inline void decode_tile(u16 data, u8* target) {
  u8 lo                    = data & 0xff;
  u8 hi                    = data >> 8;
  constexpr u8 color_map[] = {
          8,   24,  32,  //
          52,  104, 86,  //
          73,  126, 50,  //
          224, 248, 208, //
  };
  for (u8 i = 7; i > 0; i--) {
    u8 color1 = getBitN(lo, i);
    u8 color2 = getBitN(hi, i);
    u8 color  = (color2 << 1) | color1;
    GB_ASSERT(color <= 3);
    target[(7 - i) * 3 + 0] = color_map[color * 3 + 0];
    target[(7 - i) * 3 + 1] = color_map[color * 3 + 1];
    target[(7 - i) * 3 + 2] = color_map[color * 3 + 2];
  }
}

bool updateTileMap(GameBoy& gb, GLuint* out_texture, int* out_width, int* out_height) {
  constexpr u16 VRAM_BASE       = 0x8000;
  constexpr u16 MAX_TILE_COUNT  = (0x97ff - 0x8000 + 1) / 16;
  constexpr u16 TILE_PER_ROW    = 16;
  constexpr u16 TILE_PER_COL    = MAX_TILE_COUNT / 16;
  constexpr u16 WIDTH           = TILE_PER_ROW * 8;
  constexpr u16 HEIGHT          = TILE_PER_COL * 8;
  u8 pixels[WIDTH * HEIGHT * 3] = {0};
  u16 tile_idx{};

  for (u16 addr = VRAM_BASE; addr < 0x97ff; addr += 16) {
    for (u8 i = 0; i < 8; i++) {
      // 3 means RGB,
      u32 offset = (tile_idx % 16) * 8 * 3              // x offset
                   + ((tile_idx / 16) * 8) * 16 * 8 * 3 // y offset
              ;
      u16 hi = gb.memory_bus_.get(VRAM_BASE + tile_idx * 16 + i * 2);
      u16 lo = gb.memory_bus_.get(VRAM_BASE + tile_idx * 16 + i * 2 + 1);
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

  return true;
}

bool updateGameboyLCD(GameBoy& gb, GLuint* out_texture, int* out_width, int* out_height, u8 scale) {
  constexpr u32 WIDTH     = 160;
  constexpr u32 HEIGHT    = 144;
  const u32 SCALED_WIDTH  = WIDTH * scale;
  const u32 SCALED_HEIGHT = HEIGHT * scale;

  static bool init        = false;
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

  const u8* original_data = gb.ppu_.lcdData().get();
  std::vector<u8> scaled_data(SCALED_WIDTH * SCALED_HEIGHT * 4);

  for (u32 y = 0; y < HEIGHT; ++y) {
    for (u32 x = 0; x < WIDTH; ++x) {
      u32 src_index = (y * WIDTH + x) * 4;
      for (u32 dy = 0; dy < scale; ++dy) {
        for (u32 dx = 0; dx < scale; ++dx) {
          u32 dst_index = ((y * scale + dy) * SCALED_WIDTH + (x * scale + dx)) * 4;
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
  if (argc < 2) {
    GB_LOG(WARN) << "Usage: " << argv[0] << " <rom>";
    return -1;
  }

  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) return 1;

  // init gameboy
  GameBoy gb(argv[1]);
  GB_LOG(INFO) << NAMEOF_ENUM(gb.cartridge_->header().type());

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
  glfwSwapInterval(1); // Enable vsync

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


  gb.rtc_.addTask([&]() {
    if (glfwWindowShouldClose(window)) {
      gb.rtc_.stop();
    }
    return 0;
  });
  std::thread t([&]() { gb.rtc_.run(); });
  t.detach();

  auto render_loop = [&](u64) -> u8 {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
      int tile_texture_width  = 0;
      int tile_texture_height = 0;
      GLuint tile_texture_id  = 0;
      ImGui::Begin("Tile Map");
      updateTileMap(gb, &tile_texture_id, &tile_texture_width, &tile_texture_height);
      ImGui::Text("pointer = %x", tile_texture_id);
      ImGui::Text("size = %d x %d", tile_texture_width, tile_texture_height);
      ImGui::Image((void*) (intptr_t) tile_texture_id, ImVec2(tile_texture_width, tile_texture_height));
      ImGui::End();
    }

    {
      int game_texture_width  = 0;
      int game_texture_height = 0;
      static int scale        = 1;
      GLuint game_texture_id  = 0;
      ImGui::Begin("Game");
      updateGameboyLCD(gb, &game_texture_id, &game_texture_width, &game_texture_height, scale);
      ImGui::Text("pointer = %x", game_texture_id);
      ImGui::Text("size = %d x %d", game_texture_width, game_texture_height);
      ImGui::SliderInt("Scale", &scale, 1, 16);
      ImGui::Image((void*) (intptr_t) game_texture_id, ImVec2(game_texture_width, game_texture_height));
      ImGui::End();
    }
    //  Joypad status
    {
#define DEF_KEY(KEY, ACTION)                  \
  if (ImGui::IsKeyPressed(ImGuiKey_##KEY)) {  \
    gb.joypad_.ACTION(true);                  \
  }                                           \
  if (ImGui::IsKeyReleased(ImGuiKey_##KEY)) { \
    gb.joypad_.ACTION(false);                 \
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
    return 0;
  };

  // Main loop
#ifdef __EMSCRIPTEN__
  // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
  // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
  io.IniFilename = nullptr;
  EMSCRIPTEN_MAINLOOP_BEGIN
#else
  while (!glfwWindowShouldClose(window))
#endif
  {
    render_loop(1);
  }
#ifdef __EMSCRIPTEN__
  EMSCRIPTEN_MAINLOOP_END;
#endif

  // Cleanup

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
