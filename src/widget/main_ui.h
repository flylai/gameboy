#pragma once

#include <imgui.h>

#include "machine/gameboy.h"

namespace gb {

class Widgets {
public:
  void draw();

  void setGameBoy(GameBoy *gb) { gameboy_ = gb; }

private:
  bool show_tile_map_{};
  bool show_game_{true};
  bool show_cpu_registers_{};
  bool show_disassembler_{};
  void drawControlWindow();

  int scale_ = 3;
  void drawGameLCD();

  void drawTileMap();

  void drawCPURegisters();

  bool follow_pc_{};
  bool unlock_cpu_speed_{};
  void drawDisassembler();


  CircleBuffer<f32> frame_rate_buffer_{100};
  CircleBuffer<f32> cpu_speed_buffer_{100};
  bool show_frame_rate_{true};
  void drawFrameRate();

private:
  GameBoy *gameboy_{};
};

} // namespace gb
