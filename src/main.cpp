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

// clang-format off
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
// clang-format on

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

bool updateGameboyLCD(GameBoy& gb, GLuint* out_texture, int* out_width, int* out_height) {
  constexpr u32 WIDTH  = 160;
  constexpr u32 HEIGHT = 144;

  static bool init     = false;
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

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               gb.ppu_.lcdData().get());

  *out_texture = image_texture;
  *out_width   = WIDTH;
  *out_height  = HEIGHT;
  return true;
}

void data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count) {
  auto* gb = static_cast<GameBoy*>(device->pUserData);
  gb->apu_.audioDataCallback(device, output, input, frame_count);
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

  // init miniaudio
  ma_result result;
  ma_device_config deviceConfig;
  ma_device device;

  deviceConfig                   = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format   = ma_format_s16;
  deviceConfig.playback.channels = 2;
  deviceConfig.sampleRate        = ma_standard_sample_rate_44100;
  deviceConfig.dataCallback      = data_callback;
  deviceConfig.pUserData         = &gb;

  result                         = ma_device_init(nullptr, &deviceConfig, &device);
  if (result != MA_SUCCESS) {
    GB_LOG(INFO) << "Failed to initialize playback device.";
    return -1;
  }

  result = ma_device_start(&device);
  if (result != MA_SUCCESS) {
    GB_LOG(INFO) << "Failed to start playback device.";
    ma_device_uninit(&device);
    return -1;
  }


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
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Our state
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  auto render_loop   = [&](u64) -> u8 {
    static u64 last_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::high_resolution_clock::now().time_since_epoch())
                                   .count();
    u64 now = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::high_resolution_clock::now().time_since_epoch())
                      .count();
    //    if (now - last_time < 30) {
    //      return 0;
    //    }
    last_time = now;

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
      int my_image_width      = 0;
      int my_image_height     = 0;
      GLuint my_image_texture = 0;
      ImGui::Begin("Tile Map");
      updateTileMap(gb, &my_image_texture, &my_image_width, &my_image_height);
      ImGui::Text("pointer = %x", my_image_texture);
      ImGui::Text("size = %d x %d", my_image_width, my_image_height);
      ImGui::Image((void*) (intptr_t) my_image_texture, ImVec2(my_image_width, my_image_height));
      ImGui::End();
    }

    {
      int my_image_width      = 0;
      int my_image_height     = 0;
      GLuint my_image_texture = 0;
      ImGui::Begin("Game");
      updateGameboyLCD(gb, &my_image_texture, &my_image_width, &my_image_height);
      ImGui::Text("pointer = %x", my_image_texture);
      ImGui::Text("size = %d x %d", my_image_width, my_image_height);
      ImGui::Image((void*) (intptr_t) my_image_texture, ImVec2(my_image_width, my_image_height));
      ImGui::End();
    }
    //  Joypad status
    {
      ImGui::Begin("Keyboard Input");
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

      ImGui::Text("Joypad=%x", gb.joypad_.get(0xff00));
      ImGui::Text("Select=%x", gb.joypad_.select_);
      ImGui::Text("Direction=%x", gb.joypad_.direction_);
      ImGui::End();
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


  gb.rtc_.addTask([&]() {
    if (glfwWindowShouldClose(window)) {
      gb.rtc_.stop();
    }
    return 0;
  });
  std::thread t([&]() { gb.rtc_.run(); });
  t.detach();


  while (!glfwWindowShouldClose(window)) {
    render_loop(1);
  }

  // Cleanup

  ma_device_uninit(&device);

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
};
