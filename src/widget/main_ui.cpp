#include "main_ui.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "colors.h"
#include "imgui.h"

namespace gb {


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

bool updateTileMap(GameBoy* gb, GLuint* out_texture, int* out_width, int* out_height) {
#ifdef __EMSCRIPTEN__
  // WASM will throw exception about "RuntimeError: memory access out of bounds"
  return false;
#else
  if (!gb) {
    return false;
  }
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
      gb::u16 hi = gb->memory_bus_.get(VRAM_BASE + tile_idx * 16 + i * 2);
      gb::u16 lo = gb->memory_bus_.get(VRAM_BASE + tile_idx * 16 + i * 2 + 1);
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

static bool updateGameboyLCD(GameBoy* gb, GLuint* out_texture, int* out_width, int* out_height,
                             gb::u8 scale) {
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

  const gb::u8* original_data = gb->ppu_.lcdData().get();
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

void Widgets::drawControlWindow() {
  ImGui::SetNextWindowPos(ImVec2(15, 0), ImGuiCond_Once);
  ImGui::Begin("Control", nullptr, ImGuiWindowFlags_NoResize);

  ImGui::Checkbox("Frame Rate", &show_frame_rate_);
  ImGui::Checkbox("Tile Map", &show_tile_map_);
  ImGui::Checkbox("CPU Registers", &show_cpu_registers_);
  ImGui::Checkbox("Disassembler", &show_disassembler_);
  ImGui::Checkbox("Game", &show_game_);

  ImGui::End();
}

void Widgets::drawGameLCD() {
  if (!show_game_) {
    return;
  }
  ImGui::SetNextWindowSize(ImVec2(LCD_WIDTH, LCD_HEIGHT), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Size.x / 2 - ImGui::GetWindowWidth() / 2, -1),
                          ImGuiCond_Once);
  int game_texture_width  = 0;
  int game_texture_height = 0;
  GLuint game_texture_id  = 0;
  ImGui::Begin("Game", &show_game_, ImGuiWindowFlags_AlwaysAutoResize);
  updateGameboyLCD(gameboy_, &game_texture_id, &game_texture_width, &game_texture_height, scale_);
  ImGui::Text("pointer = %x", game_texture_id);
  ImGui::Text("size = %d x %d", game_texture_width, game_texture_height);
  ImGui::SliderInt("Scale", &scale_, 1, 16);
  ImGui::Image((void*) (intptr_t) game_texture_id, ImVec2(game_texture_width, game_texture_height));
  ImGui::End();
}

void Widgets::drawTileMap() {
  if (!show_tile_map_) {
    return;
  }
  int tile_texture_width  = 0;
  int tile_texture_height = 0;
  GLuint tile_texture_id  = 0;

  ImGui::SetNextWindowPos(ImVec2(180, 145), ImGuiCond_Once);
  ImGui::Begin("Tile Map", &show_tile_map_, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
  updateTileMap(gameboy_, &tile_texture_id, &tile_texture_width, &tile_texture_height);
  ImGui::Text("pointer = %x", tile_texture_id);
  ImGui::Text("size = %d x %d", tile_texture_width, tile_texture_height);
  ImGui::Image((void*) (intptr_t) tile_texture_id, ImVec2(tile_texture_width, tile_texture_height));
  ImGui::End();
}

static const char* FMT[] = {
        "",      "",      "%s", "", //
        "",      "",      "",   "", //
        "%o",    "",      "%d", "", //
        "",      "",      "",   "", //
        "$%02X", "$%04X",
};

void Widgets::drawCPURegisters() {
  if (!show_cpu_registers_) {
    return;
  }

  static int v       = 16;
  const auto& cpu    = gameboy_->cpu_;
  const auto& memory = gameboy_->memory_bus_;
  ImGui::SetNextWindowPos(ImVec2(15, 145), ImGuiCond_Once);
  ImGui::Begin("CPU Registers", &show_cpu_registers_,
               ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);

  ImGui::SeparatorText("Data Type");
  ImGui::RadioButton("Hex", &v, 16);
  ImGui::SameLine();
  ImGui::RadioButton("Dec", &v, 10);
  ImGui::SameLine();
  ImGui::RadioButton("Oct", &v, 8);

  {
    ImGui::BeginTable("ZNHC", 4, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Z");
    ImGui::TableSetupColumn("N");
    ImGui::TableSetupColumn("H");
    ImGui::TableSetupColumn("C");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::TextColored(color::Aqua, "%d", cpu.zf());
    ImGui::TableSetColumnIndex(1);
    ImGui::TextColored(color::Aqua, "%d", cpu.nf());
    ImGui::TableSetColumnIndex(2);
    ImGui::TextColored(color::Aqua, "%d", cpu.hf());
    ImGui::TableSetColumnIndex(3);
    ImGui::TextColored(color::Aqua, "%d", cpu.cf());
    ImGui::EndTable();
  }

  {
    ImGui::BeginTable("AF", 3, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("A");
    ImGui::TableSetupColumn("F");
    ImGui::TableSetupColumn("AF");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::TextColored(color::AntiqueWhite, FMT[v], cpu.A());
    ImGui::TableSetColumnIndex(1);
    ImGui::TextColored(color::AntiqueWhite, FMT[v], cpu.F());
    ImGui::TableSetColumnIndex(2);
    ImGui::TextColored(color::DeepSkyBlue, FMT[v == 16 ? v + 1 : v], cpu.AF());
    ImGui::EndTable();
  }
  {
    ImGui::BeginTable("BC", 3, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("B");
    ImGui::TableSetupColumn("C");
    ImGui::TableSetupColumn("BC");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::TextColored(color::AntiqueWhite, FMT[v], cpu.B());
    ImGui::TableSetColumnIndex(1);
    ImGui::TextColored(color::AntiqueWhite, FMT[v], cpu.C());
    ImGui::TableSetColumnIndex(2);
    ImGui::TextColored(color::DeepSkyBlue, FMT[v == 16 ? v + 1 : v], cpu.BC());
    ImGui::EndTable();
  }
  {
    ImGui::BeginTable("DE", 3, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("D");
    ImGui::TableSetupColumn("E");
    ImGui::TableSetupColumn("DE");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::TextColored(color::AntiqueWhite, FMT[v], cpu.D());
    ImGui::TableSetColumnIndex(1);
    ImGui::TextColored(color::AntiqueWhite, FMT[v], cpu.E());
    ImGui::TableSetColumnIndex(2);
    ImGui::TextColored(color::DeepSkyBlue, FMT[v == 16 ? v + 1 : v], cpu.DE());
    ImGui::EndTable();
  }
  {
    ImGui::BeginTable("HL", 3, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("H");
    ImGui::TableSetupColumn("L");
    ImGui::TableSetupColumn("HL");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::TextColored(color::AntiqueWhite, FMT[v], cpu.B());
    ImGui::TableSetColumnIndex(1);
    ImGui::TextColored(color::AntiqueWhite, FMT[v], cpu.C());
    ImGui::TableSetColumnIndex(2);
    ImGui::TextColored(color::DeepSkyBlue, FMT[v == 16 ? v + 1 : v], cpu.HL());
    ImGui::EndTable();
  }

  {
    ImGui::BeginTable("SP,PC", 2, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("SP");
    ImGui::TableSetupColumn("PC");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::TextColored(color::DeepSkyBlue, FMT[v == 16 ? v + 1 : v], cpu.SP());
    ImGui::TableSetColumnIndex(1);
    ImGui::TextColored(color::DeepSkyBlue, FMT[v == 16 ? v + 1 : v], cpu.PC());
    ImGui::EndTable();
  }

  {
    ImGui::BeginTable("Interrupt", 3, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("IME");
    ImGui::TableSetupColumn("IE");
    ImGui::TableSetupColumn("IF");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::TextColored(color::Aqua, "%d", cpu.IME());
    ImGui::TableSetColumnIndex(1);
    ImGui::TextColored(color::DeepSkyBlue, FMT[v == 16 ? v + 1 : v], memory.get(IE_BASE));
    ImGui::TableSetColumnIndex(2);
    ImGui::TextColored(color::DeepSkyBlue, FMT[v == 16 ? v + 1 : v], memory.get(IF_BASE));
    ImGui::EndTable();
  }


  ImGui::End();
}

void Widgets::drawDisassembler() {
  if (!show_disassembler_) {
    gameboy_->cpu_.disassembler().disable();
    return;
  }
  gameboy_->cpu_.disassembler().enable();

  ImGui::SetNextWindowPos(ImVec2(950, 180), ImGuiCond_Once);
  ImGui::SetNextWindowSize(ImVec2(315, 400), ImGuiCond_Once);
  ImGui::Begin("Disassembler", &show_disassembler_, ImGuiWindowFlags_NoScrollbar);

  ImGui::Checkbox("Follow PC", &follow_pc_);
  ImGui::SameLine();
  ImGui::Checkbox("Unlock CPU Speed", &unlock_cpu_speed_);
  gameboy_->rtc_.cpuSpeedLock(unlock_cpu_speed_);


  if (ImGui::Button("Continue")) {
    gameboy_->rtc_.resume();
  }
  ImGui::SameLine();
  if (ImGui::Button("Pause")) {
    gameboy_->rtc_.pause();
  }
  ImGui::SameLine();
  if (ImGui::Button("Step")) {
    gameboy_->rtc_.pause();
    if (gameboy_->rtc_.paused()) {
      gameboy_->cpu_.update();
    }
  }


  ImGui::BeginChild("ASM", ImVec2(0, 300), ImGuiChildFlags_Border, 0);
  const auto& disassembler = gameboy_->cpu_.disassembler();
  u16 pc                   = gameboy_->cpu_.PC();
  ImGuiListClipper clipper;

  u16 record_count = disassembler.recordCount();
  [[maybe_unused]] u16 cond{};
  u32 pc_item_idx{};
  clipper.Begin(record_count);
  // make a copy. do not use the origin data directly
  std::vector<const DisassembleRecord*> records_copy(record_count);
  for (u16 i = 0, j = 0; i < record_count;) {
    while (disassembler.records()[j].instruction == nullptr) {
      j++;
    }
    const auto& record = disassembler.records()[j];

    if (pc == record.pc) {
      pc_item_idx = i;
    }

    if (record.pc < pc && Disassembler::isBasicBlockEnd(record.instruction->idx)) {
      cond++;
    }
    records_copy[i++] = &record;
    j++;
  }


  if (follow_pc_) {
    float line_height   = ImGui::GetTextLineHeightWithSpacing();

    // scroll to middle
    float window_offset = ImGui::GetWindowHeight() / 2.0f;
    float offset        = window_offset - line_height / 2.0f;

    float current_y     = ImGui::GetScrollY();
    float item_offset   = pc_item_idx * line_height;

    if (item_offset < current_y || item_offset > current_y + ImGui::GetWindowHeight()) {
      ImGui::SetScrollY(item_offset - offset);
    }
  }

  while (clipper.Step()) {
    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
      const auto* record = records_copy[i];
      bool is_pc_inst    = pc == record->pc;

      ImGui::PushID(i);
      ImGui::Selectable("");
      ImGui::SameLine();

      // address
      ImGui::TextColored(is_pc_inst ? color::Yellow : color::LightGreen, "%04X", record->pc);
      ImGui::SameLine();

      // machine code
      if (is_pc_inst) {
        ImGui::TextColored(color::Yellow, " -> %02X", record->instruction->idx);
      } else {
        ImGui::TextColored(color::LightSteelBlue, "    %02X", record->instruction->idx);
      }
      ImGui::SameLine();

      // operand
      for (u8 j = 0; j < record->instruction->size - 1; j++) {
        ImGui::TextColored(color::Gray, "%02X", record->operand[j]);
        ImGui::SameLine();
      }
      // padding
      ImGui::Text("%*s", 8 - (record->instruction->size - 1) * 3, "");
      ImGui::SameLine();

      ImGui::TextColored(is_pc_inst ? color::Yellow : color::White, "%s", record->instruction->name);

      if (Disassembler::isBasicBlockEnd(record->instruction->idx)) {
        // TODO: if add the separator, the PC item offset will be wrong.
        // I dont know the height of the separator.
        // ImGui::Separator();
      }

      ImGui::PopID(); // ImGui::PushID(i)
    }
  }

  clipper.End();

  ImGui::EndChild();
  ImGui::End();
}

void Widgets::drawFrameRate() {
  if (!show_frame_rate_) {
    return;
  }

  auto circle_buffer_getter = [](void* buffer, int idx) -> f32 {
    return ((CircleBuffer<f32>*) buffer)->operator[](idx);
  };

  f32 frameRate = ImGui::GetIO().Framerate;
  frame_rate_buffer_.push(frameRate);
  cpu_speed_buffer_.push((f32) gameboy_->rtc_.cpuSpeed());

  ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Size.x - 330, -1), ImGuiCond_Once);
  ImGui::Begin("Frame Rate", &show_frame_rate_, ImGuiWindowFlags_AlwaysAutoResize);


  char buf[80];

  sprintf(buf, "Render Frame Rate: %.1f FPS", frameRate);
  ImGui::PlotLines("", circle_buffer_getter, &frame_rate_buffer_, frame_rate_buffer_.size(), 0, buf, 30.0f,
                   240.0f, ImVec2(300, 70));

  sprintf(buf, "CPU Speed: %lu Hz", gameboy_->rtc_.cpuSpeed());
  ImGui::PlotLines("", circle_buffer_getter, &cpu_speed_buffer_, cpu_speed_buffer_.size(), 0, buf, 1000000.f,
                   40000000.f, ImVec2(300, 70));

  ImGui::End();
}

void Widgets::draw() {
  drawControlWindow();
  drawTileMap();
  drawGameLCD();
  drawCPURegisters();
  drawDisassembler();
  drawFrameRate();
}

} // namespace gb
