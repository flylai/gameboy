#pragma once

#include <imgui.h>
#include <imgui_memory_editor/imgui_memory_editor.h>

#include "machine/gameboy.h"

namespace gb {

class Widgets {
public:
  void draw();

  void setGameBoy(GameBoy* gb) {
    gameboy_                = gb;
    memory_editor_.ReadFn   = ReadFn_;
    memory_editor_.WriteFn  = WriteFn_;
    memory_editor_.UserData = gameboy_;
  }

private:
  bool show_tile_map_{};
  bool show_game_{true};
  bool show_cpu_registers_{};
  bool show_disassembler_{};
  void drawControlWindow();

  int scale_ = 3;
  int palette_names_idx_{};
  void drawGameLCD();

  void drawTileMap();

  void drawCPURegisters();

  bool follow_pc_{};
  bool unlock_cpu_speed_{};
  void drawDisassembler();

  // Memory editor
  bool show_memory_editor_{};
  MemoryEditor memory_editor_{};

  static ImU8 ReadFn_(const ImU8* data, size_t off, void* user_data) {
    return ((GameBoy*) user_data)->memory_bus_.get(off);
  }

  static void WriteFn_(ImU8* data, size_t off, ImU8 d, void* user_data) {
    ((GameBoy*) user_data)->memory_bus_.set(off, d);
  }

  void drawMemoryViewer();

  CircleBuffer<f32> frame_rate_buffer_{100};
  CircleBuffer<f32> cpu_speed_buffer_{100};
  bool show_frame_rate_{true};
  void drawFrameRate();

private:
  GameBoy* gameboy_{};
};

} // namespace gb
